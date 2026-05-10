#include <thread>
#include <iostream>

class jthread
{
    std::thread t;

public:
    template <typename F, typename... Args>
    explicit jthread(F &&f, Args &&...) : t(std::forward<F>(f), std::forward<Args>(args)...)
    {
    }
    explicit jthread(std::thread &&t_) noexcept : t(std::move(t_))
    {
    }
    jthread(jthread &&other) : t(std::move(other.t))
    {
    }
    jthread &operator=(jthread &&other) noexcept
    {
        if (joinable())
            join();

        t = std::move(other.t);
        return *this;
    }

    jthread &operator=(std::thread other) noexcept
    {
        if (joinable())
            join();
        t = std::move(other);
        return *this;
    }
    ~jthread() noexcept
    {
        if (joinable())
            join();
    }
    void swap(jthread &other) noexcept
    {
        t.swap(other.t);
    }
    std::thread::id get_id() const noexcept
    {
        return t.get_id();
    }
    bool joinable() const noexcept
    {
        return t.joinable();
    }
    void join()
    {
        t.join();
    }
    void detach()
    {
        t.detach();
    }
    std::thread &as_thread() noexcept
    {
        return t;
    }
    const std::thread &as_thread() const noexcept
    {
        return t;
    }
};
