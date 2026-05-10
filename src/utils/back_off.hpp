#ifndef BACK_OFF_H
#define BACK_OFF_H

#include <thread>
#include <chrono>
#include <random>
#include <algorithm>

struct back_off
{
    const size_t min_delay, max_delay;
    size_t limit;
    std::random_device rd;
    static thread_local std::mt19937 rng;

    back_off(size_t min, size_t max) : min_delay(min), max_delay (max),
        limit(min){}

    void back_off_thread() 
    {
        std::uniform_int_distribution<int> dist(0, limit);
        size_t delay = dist(rng);
        limit = std::min(limit * 2, max_delay);

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }    

};

thread_local std::mt19937 back_off::rng{std::random_device{}()};


#endif
