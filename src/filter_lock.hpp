#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

// Maps each thread to a simple integer ID
thread_local int thread_id = -1;

class base_lock {
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
};

class filter_lock : public base_lock {
public:
    int* level;
    int* victim;
    int n;

    filter_lock(int n_) : n(n_) {       // fix: initialize n first via initializer list
        level = new int[n]();            // () zero-initializes the array
        victim = new int[n]();
    }

    ~filter_lock() {
        delete[] level;
        delete[] victim;
    }

    void lock() override {
        int me = thread_id;              // use our manual thread ID
        for (int i = 1; i < n; i++) {
            level[me] = i;
            victim[i] = me;

            bool waiting = true;
            while (waiting) {
                waiting = false;
                for (int k = 0; k < n; k++) {
                    if (k != me && level[k] >= i && victim[i] == me) {
                        waiting = true;
                        break;
                    }
                }
            }
        }
    }

    void unlock() override {
        level[thread_id] = 0;
    }
};

