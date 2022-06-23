#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <queue>
#include <vector>

#include "profile.h"

namespace {

using VectorIt = std::vector<int>::iterator;

}

namespace {

std::vector<int> GenerateBigRandomVector(size_t size) {
    std::vector<int> big_vector;
    big_vector.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        big_vector.push_back(std::rand());
    }

    return big_vector;
}

template<typename Iter>
void AsyncParallelMergeSort(Iter begin, Iter end,
                   unsigned int N = std::thread::hardware_concurrency())
{
    auto len = std::distance(begin, end);
    if (len <= 1024)
    {
        std::sort(begin,end);
        return;
    }
    
    Iter mid = std::next(begin, len/2);
    if (N > 1)
    {
        auto fn = std::async(AsyncParallelMergeSort<Iter>, begin, mid, N-2);
        AsyncParallelMergeSort(mid, end, N-2);
        fn.wait();
    }
    else
    {
        AsyncParallelMergeSort(begin, mid, 0);
        AsyncParallelMergeSort(mid, end, 0);
    }
    
    std::inplace_merge(begin, mid, end);
}

template<typename Iter>
void ThreadsParallelMergeSort(Iter begin, Iter end,
                   unsigned int N = std::thread::hardware_concurrency())
{
    auto len = std::distance(begin, end);
    if (len <= 1024)
    {
        std::sort(begin,end);
        return;
    }
    
    Iter mid = std::next(begin, len/2);
    if (N > 1)
    {
        auto fn = std::thread(ThreadsParallelMergeSort<Iter>, begin, mid, N-2);
        ThreadsParallelMergeSort(mid, end, N-2);
        fn.join();
    }
    else
    {
        ThreadsParallelMergeSort(begin, mid, 0);
        ThreadsParallelMergeSort(mid, end, 0);
    }
    
    std::inplace_merge(begin, mid, end);
}

}

int main(int argc, char **argv) {    
    const size_t kVectorSize = std::stoi(argv[1]);
    const size_t kParallelThreadsCount = std::stoi(argv[2]);

    std::cout << "Vector size: " << kVectorSize
              << ", threads number: " << kParallelThreadsCount << "\n";

    const auto big_vector = GenerateBigRandomVector(kVectorSize);
    {
        LOG_DURATION("one-threaded sort");
        auto vector_copy = big_vector;
        std::sort(vector_copy.begin(), vector_copy.end());
    }
    {
        LOG_DURATION("multi-threaded async sort");
        auto vector_copy = big_vector;
        AsyncParallelMergeSort(vector_copy.begin(), vector_copy.end(), kParallelThreadsCount);
    }
    {
        LOG_DURATION("multi-threaded raw threads sort");
        auto vector_copy = big_vector;
        ThreadsParallelMergeSort(vector_copy.begin(), vector_copy.end(), kParallelThreadsCount);
    }

    return 0;
}
