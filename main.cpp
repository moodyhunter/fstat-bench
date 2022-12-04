#include <chrono>
#include <cstdio>
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

template<size_t N>
std::chrono::microseconds timed_run(const std::string (&strings)[N])
{
    int result = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < N; ++i)
    {
        std::cout << strings[i] << std::endl;
        result += stat(strings[i].c_str(), s);
    }
    auto end = std::chrono::high_resolution_clock::now();

    if (unlikely(result != 0))
    {
        std::cerr << "stat failed" << std::endl;
        return std::chrono::microseconds(0);
    }

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

    std::string strings[MAX_FILES] = { 0 }; // 1000 files
    size_t i = 0;
    while (file.good() && i < MAX_FILES)
    {
        std::getline(file, strings[i++]);
    }

    auto duration = timed_run(strings);
    std::cout << "duration: " << duration.count() << "us" << std::endl;

    return 0;
}
