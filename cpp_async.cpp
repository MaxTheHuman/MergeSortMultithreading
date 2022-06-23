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

void AsyncParallelMergeSort(std::vector<int>& numbers, size_t parallel_threads_number) {
    auto begin_it = numbers.begin();
    const auto numbers_end_it = numbers.end();
    const auto part_size = numbers.size() / parallel_threads_number;
    std::vector<std::future<void>> futures;
    std::vector<std::pair<VectorIt, VectorIt>> parts_iters;
    for (size_t i = 0; i < parallel_threads_number; ++i) {
        auto end_it = (i == parallel_threads_number - 1)
                        ? numbers_end_it
                        : begin_it + part_size;
        futures.push_back(std::async(std::launch::async, [begin_it, end_it] {
                    std::sort(begin_it, end_it);
                }
            )
        );
        begin_it += part_size;
        parts_iters.push_back(std::make_pair(begin_it, end_it));
    }

    auto comp = [](
        const std::pair<VectorIt, VectorIt>& left,
        const std::pair<VectorIt, VectorIt>& right) {
          return *left.first > *right.first;
        };
    std::priority_queue<std::pair<VectorIt, VectorIt>,
        std::vector<std::pair<VectorIt, VectorIt>>, decltype(comp)> iters_heap(comp);
    for (const auto& part_iters: parts_iters) {
        iters_heap.push(part_iters);
    }

    for (auto& future : futures) {
        future.get();
    }

    std::vector<int> result;
    result.reserve(numbers.size());
    while (!iters_heap.empty()) {
        const auto curr_iters = iters_heap.top();
        iters_heap.pop();
        result.push_back(*curr_iters.first);
        if (curr_iters.first != curr_iters.second) {
            iters_heap.push(std::make_pair(curr_iters.first + 1, curr_iters.second));
        }
    }

    numbers = result;
}

void ThreadsParallelMergeSort(std::vector<int>& numbers, size_t parallel_threads_number) {
    auto begin_it = numbers.begin();
    const auto numbers_end_it = numbers.end();
    const auto part_size = numbers.size() / parallel_threads_number;
    std::vector<std::thread> threads;
    std::vector<std::pair<VectorIt, VectorIt>> parts_iters;
    for (size_t i = 0; i < parallel_threads_number; ++i) {
        auto end_it = (i == parallel_threads_number - 1)
                        ? numbers_end_it
                        : begin_it + part_size;
        threads.push_back(std::thread([begin_it, end_it] {
                    std::sort(begin_it, end_it);
                }
            )
        );
        begin_it += part_size;
        parts_iters.push_back(std::make_pair(begin_it, end_it));
    }

    auto comp = [](
        const std::pair<VectorIt, VectorIt>& left,
        const std::pair<VectorIt, VectorIt>& right) {
          return *left.first > *right.first;
        };
    std::priority_queue<std::pair<VectorIt, VectorIt>,
        std::vector<std::pair<VectorIt, VectorIt>>, decltype(comp)> iters_heap(comp);
    for (const auto& part_iters: parts_iters) {
        iters_heap.push(part_iters);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::vector<int> result;
    result.reserve(numbers.size());
    while (!iters_heap.empty()) {
        const auto curr_iters = iters_heap.top();
        iters_heap.pop();
        result.push_back(*curr_iters.first);
        if (curr_iters.first != curr_iters.second) {
            iters_heap.push(std::make_pair(curr_iters.first + 1, curr_iters.second));
        }
    }

    numbers = result;
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
        AsyncParallelMergeSort(vector_copy, kParallelThreadsCount);
    }
    {
        LOG_DURATION("multi-threaded raw threads sort");
        auto vector_copy = big_vector;
        ThreadsParallelMergeSort(vector_copy, kParallelThreadsCount);
    }

    return 0;
}