#include <iostream>
#include <mutex>
#include <chrono>
#include <random>
#include <thread>
#include <vector>
#include <functional>

using namespace std;

const size_t N = 5;

int myrand(int min, int max) {
  static mt19937 rnd(time(nullptr));
  return uniform_int_distribution<>(min,max)(rnd);
}

void take_action (size_t id, mutex &ma, mutex& mb, mutex& co)
{
    while (true)
    {
        int duration = myrand(200, 800);
        {
            lock_guard<mutex> gco (co);
            cout<<"Philosopher "<<id<<" is thinking \n";
        }
        this_thread::sleep_for(chrono::milliseconds(duration));

        {
            lock_guard<mutex>gco (co);
            cout<<"Philosopher "<<id<<" is hungry \n";
        }
        lock_guard<mutex> gma(ma);
        lock_guard<mutex> gmb(mb);
        duration = myrand(200, 800);
        {
            lock_guard<mutex> gco(co);
            cout<<"Philosopher "<<id<<" is eating \n";
        }
        this_thread::sleep_for(chrono::milliseconds(duration));
    }
}

int main () 
{
    mutex forks [N];
    mutex co;
    vector<thread> threads;
    threads.reserve(N);
    for(int i = 0; i < N; i++)
    {
        threads.emplace_back(
            take_action,
            i,
            ref(forks[i]),
            ref(forks[(i + 1) % N]),
            ref(co)
        );
    }
    
    for(auto &t : threads)
    {
        t.join();
    }

    return 0;
}
