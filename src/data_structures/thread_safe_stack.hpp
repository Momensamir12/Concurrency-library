#include <memory>
#include <exception>
#include <stack>
#include <mutex>

struct empty_stack : std::exception
{
    const char *what() const noexcept;
};

template <typename T>
class thread_safe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    thread_safe_stack()
    {
    }
    thread_safe_stack(const thread_safe_stack &other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }
    void push(T val)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(val);
    }

    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty())
            throw empty_stack();

        std::shared_ptr<T> const val(std::make_shared<T>(data.top()));

        return val;
    }
    bool empty const()
    {
        std::lock_guard<std::mutex> lock(m);
        // m.lock();
        // bool empty = data.empty();
        // m.unlock();
        // return empty;
        return data.empty();
    }
};
