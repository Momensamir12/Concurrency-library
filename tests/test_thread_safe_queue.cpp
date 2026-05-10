#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "data_structures/thread_safe_queue.hpp"

int main()
{
    thread_safe_queue<int> q;

    const int producers = 4;
    const int consumers = 3;
    const int items_per_producer = 1000;
    const int total_items = producers * items_per_producer;

    std::vector<int> counts(total_items, 0);
    std::mutex counts_mutex;
    std::atomic<int> consumed{0};

    std::vector<std::thread> producer_threads;
    producer_threads.reserve(producers);

    for (int p = 0; p < producers; ++p)
    {
        producer_threads.emplace_back([p, items_per_producer, &q]() {
            const int base = p * items_per_producer;
            for (int i = 0; i < items_per_producer; ++i)
            {
                q.push(base + i);
            }
        });
    }

    std::vector<std::thread> consumer_threads;
    consumer_threads.reserve(consumers);

    for (int c = 0; c < consumers; ++c)
    {
        consumer_threads.emplace_back([&]() {
            while (true)
            {
                int value = 0;
                q.wait_and_pop(value);
                if (value < 0)
                {
                    break;
                }

                {
                    std::lock_guard<std::mutex> lock(counts_mutex);
                    counts[value] += 1;
                }

                consumed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto &t : producer_threads)
    {
        t.join();
    }

    for (int i = 0; i < consumers; ++i)
    {
        q.push(-1);
    }

    for (auto &t : consumer_threads)
    {
        t.join();
    }

    bool ok = (consumed.load() == total_items);
    for (int i = 0; i < total_items; ++i)
    {
        if (counts[i] != 1)
        {
            ok = false;
            break;
        }
    }

    if (!q.empty())
    {
        ok = false;
    }

    if (ok)
    {
        std::cout << "thread_safe_queue: PASS" << std::endl;
        return 0;
    }

    std::cout << "thread_safe_queue: FAIL" << std::endl;
    return 1;
}
