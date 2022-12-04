#include <chrono>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <new>
#include <sys/stat.h>
#include <type_traits>
#include <unistd.h>

#define unlikely(x) __builtin_expect(!!(x), 0)

constexpr auto MAX_SUPPORTED_FILES = 100000000;

std::pair<std::chrono::microseconds, size_t> timed_run(const std::string *strings, const size_t N)
{
    constexpr auto FD_CACHE_SIZE = 1024;
    struct stat *s = new (std::align_val_t(4096)) struct stat;

    std::chrono::microseconds total_time(0);
    size_t total_files = 0;
    int *fd_cache = new (std::align_val_t(4096)) int[FD_CACHE_SIZE]{ -1 };

    std::cout << "Using a batch size of " << FD_CACHE_SIZE << std::endl;

    const std::string *next_string = strings;
    size_t batch_idx = 0;
    while (next_string < strings + N)
    {
        if (batch_idx % 20 == 0)
            std::cout << "Processing batch " << batch_idx << std::endl;
        batch_idx++;
        size_t valid_fd_count = 0;
        for (valid_fd_count = 0; valid_fd_count < FD_CACHE_SIZE && (next_string < strings + N); next_string++)
        {
            if (next_string->empty())
                continue;
            int fd = open(next_string->c_str(), O_NOFOLLOW | O_PATH);
            if (unlikely(fd == -1))
            {
                std::cerr << "Ignored file: " << *next_string << " (" << strerror(errno) << ")" << std::endl;
                continue;
            }
            fd_cache[valid_fd_count++] = fd;
        }

        const auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < valid_fd_count; ++i)
        {
            const auto result = fstat(fd_cache[i], s);
            if (unlikely(result != 0))
                std::cerr << "stat failed: " << std::strerror(errno) << " for " << strings[i] << std::endl;
            else
                total_files++;
        }
        const auto end = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < valid_fd_count; ++i)
            close(fd_cache[i]);

        total_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }

    delete[] fd_cache;
    delete s;
    return { total_time, total_files };
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " list_of_files.txt" << std::endl;
        return 1;
    }

    std::fstream file(argv[1]);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file " << argv[1] << std::endl;
        return 1;
    }

    std::cout << "Allocating memory..." << std::endl;
    std::string *strings = new std::string[MAX_SUPPORTED_FILES];
    size_t i = 0;
    while (file.good() && i < MAX_SUPPORTED_FILES)
    {
        std::getline(file, strings[i++]);
    }

    std::cout << "Starting test `stat` on " << i << " files" << std::endl;

    auto [duration, files] = timed_run(strings, i);
    delete[] strings;

    std::cout << "Processed " << files << " files in " << duration.count() << " microseconds" << std::endl;
    std::cout << "  avg: " << (long double) duration.count() / files << " microseconds (or " << files * 1000000 / duration.count() << " files per second)" << std::endl;

    return 0;
}
