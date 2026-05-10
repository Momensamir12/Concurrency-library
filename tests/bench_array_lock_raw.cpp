#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#include "locks/queue_lock.hpp"

namespace {

struct BenchResult {
    double seconds;
    std::uint64_t total_ops;
    std::uint64_t final_count;
};

BenchResult run_benchmark(int threads, std::uint64_t iterations)
{
    array_lock lock(threads);
    std::uint64_t counter = 0;

    std::vector<std::thread> workers;
    workers.reserve(static_cast<std::size_t>(threads));

    auto start = std::chrono::steady_clock::now();
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&]() {
            for (std::uint64_t i = 0; i < iterations; ++i) {
                lock.lock();
                ++counter;
                lock.unlock();
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::uint64_t total_ops = iterations * static_cast<std::uint64_t>(threads);

    std::cout << "array_lock (raw flags): " << elapsed.count() << " s, "
              << (static_cast<double>(total_ops) / elapsed.count()) << " ops/s, "
              << "count=" << counter << '\n';

    return {elapsed.count(), total_ops, counter};
}

} // namespace

int main(int argc, char** argv)
{
    int threads = static_cast<int>(std::thread::hardware_concurrency());
    if (threads <= 0) {
        threads = 4;
    }

    std::uint64_t iterations = 1'000'000;

    if (argc >= 2) {
        threads = std::max(1, std::atoi(argv[1]));
    }
    if (argc >= 3) {
        iterations = static_cast<std::uint64_t>(std::strtoull(argv[2], nullptr, 10));
        if (iterations == 0) {
            iterations = 1;
        }
    }

    std::cout << "Threads: " << threads << ", iterations/thread: " << iterations << '\n';

    run_benchmark(threads, iterations);

    return 0;
}
