#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
template <typename T>
class thread_safe_queue
{
private:
    mutable std::mutex m;
    std::queue<T> q;
    std::condition_variable cv;
    
public:
    thread_safe_queue()
    {
        
    }
    thread_safe_queue (thread_safe_queue const& other)
    {
        std::lock_guard<std::mutex>lock (other.m);
        q = other.q;
    }

    void push (T val)
    {
        std::lock_guard<std::mutex>lock (m);
        q.push(val);
        cv.notify_one();
    }

    bool try_pop (T& value)
    {
        std::lock_guard<std::mutex>lock (m);
        if(q.empty())
           return false;

        value = q.front();
        q.pop();
        
        return true;   
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex>lock (m);
        cv.wait(lock, [this] {return !q.empty();});

        value = q.front();
        q.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex>lock (m);
        cv.wait(lock, [this] {return !q.empty();});

        std::shared_ptr<T> val (std::make_shared<T>(q.front()));
        q.pop();

        return val;
    }

    bool empty () const
    {
        std::lock_guard<std::mutex> lock(m);
        return q.empty();
    }
}; 


#endif
