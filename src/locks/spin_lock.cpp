#pragma once

#include <iostream>
#include <atomic>
#include "back_off.hpp"

class test_and_set_lock
{
    private:
    static constexpr size_t min_delay = 10;
    static constexpr size_t max_delay = 10000;
    std::atomic<bool> state{false};
    
    void lock()
    {
        while (true)
        {
            back_off back_off(min_delay, max_delay);
            // why use acquire and release here ? 
            // we need to prevent critical section instructions from getting 
            // reordered before or after acquiring the lock
            // acquire : no instructions after me get ordered before me
            // release : no instructions before me get ordered after me
            // default is seq_cst which is too strong 
            while (state.load(std::memory_order_relaxed)) // we aren't taking the lock here so we can use relaxed              
            {
            }
            if (!state.exchange(true, std::memory_order_acquire))
            {
                return;
            }
            else
            {
                back_off.back_off_thread();
            }
        }
    }
    void unlock()
    {
        state.store(false, std::memory_order_release);
    }
};
