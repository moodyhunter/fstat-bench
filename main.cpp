#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <new>
#include <sys/stat.h>
#include <type_traits>

#define unlikely(x) __builtin_expect(!!(x), 0)

struct stat *s = new (std::align_val_t(4096)) struct stat;

constexpr auto MAX_FILES = 1000000;

std::chrono::microseconds timed_run(const std::string *strings, const size_t N)
{
    const auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; ++i)
    {
        const auto result = stat(strings[i].c_str(), s);
        if (unlikely(result != 0))
            std::cerr << "stat failed: " << std::strerror(errno) << " for " << strings[i] << std::endl;
    }
    const auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
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

    std::string *strings = new std::string[MAX_FILES];
    size_t i = 0;
    while (file.good() && i < MAX_FILES)
    {
        std::getline(file, strings[i++]);
    }

    std::cout << "Starting test `stat` on " << i << " files" << std::endl;

    auto duration = timed_run(strings, i);
    std::cout << "duration: " << duration.count() << "us" << std::endl;

    return 0;
}
