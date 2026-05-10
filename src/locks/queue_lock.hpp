#pragma once

#include <atomic>
#include <thread>
#include <utils/cache_aligned_data.hpp>

struct array_lock_cached
{
private:    
    int size;
    std::atomic<int> tail{0};
    thread_local static int slot_index;
    cache_aligned_data<std::atomic<bool>>* flag;

public:
    explicit array_lock_cached (int capacity) : size(capacity)
    {
        flag = new cache_aligned_data<std::atomic<bool>> [capacity];
        flag[0].value.store(true, std::memory_order_relaxed);
    }
    ~array_lock_cached ()
    {
        delete[] flag;
    }
    void lock()
    {
        slot_index = tail.fetch_add(1, std::memory_order_relaxed) % size;

        while(!flag[slot_index].value.load(std::memory_order_acquire))
        {

        }
    }
    void unlock()
    {
        flag[slot_index].value.store(false, std::memory_order_release);
        flag[(slot_index + 1) % size].value.store(true, std::memory_order_release);
    }
};
thread_local int array_lock_cached::slot_index = 0;

struct array_lock
{
private:    
    int size;
    std::atomic<int> tail{0};
    thread_local static int slot_index;
    std::atomic<bool>* flag;

public:
    explicit array_lock (int capacity) : size(capacity)
    {
        flag = new std::atomic<bool> [capacity];
        flag[0].store(true, std::memory_order_relaxed);
    }
    ~array_lock ()
    {
        delete[] flag;
    }
    void lock()
    {
        slot_index = tail.fetch_add(1, std::memory_order_relaxed) % size;

        while(!flag[slot_index].load(std::memory_order_acquire))
        {

        }
    }
    void unlock()
    {
        flag[slot_index].store(false, std::memory_order_release);
        flag[(slot_index + 1) % size].store(true, std::memory_order_release);
    }
};
thread_local int array_lock::slot_index = 0;

