#include "filter_lock.hpp"
#include <memory>
#include <mutex>
#include <stdexcept>

template<typename T>
class lock_based_circular_buffer 
{
private:
    std::unique_ptr<base_lock> lock_;
    std::unique_ptr<T[]> buffer_;
    int head, tail;
    size_t size_;

public:    
    explicit lock_based_circular_buffer(size_t size)
        : lock_(std::make_unique<filter_lock>(2)),
          buffer_(std::make_unique<T[]>(size)),
          head(0),
          tail(0),
          size_(size)
    {
    }

    void enq(T data)
    {
        std::lock_guard<base_lock> guard(*lock_);

        if (tail - head == static_cast<int>(size_))
            throw std::runtime_error("Buffer is full");

        buffer_[tail % size_] = data;
        tail++;
    }
    public T deq()
    {
        std::lock_guard<base_lock> guard(*lock_);
        if(tail == head)
            throw std::runtime_error("Buffer is empty");

        T x = buffer_[head % size_];
        head++;
        
        return x;
    }

};
