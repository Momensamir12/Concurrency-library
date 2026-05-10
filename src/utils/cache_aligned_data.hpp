#pragma once

#include <cstddef>
#include <new>

namespace cache_aligned_detail {
#if defined(__cpp_lib_hardware_interference_size)
constexpr std::size_t cache_line_size = std::hardware_destructive_interference_size;
#else
constexpr std::size_t cache_line_size = 64;
#endif
}

template<typename T>
struct alignas(cache_aligned_detail::cache_line_size) cache_aligned_data
{
    T value{};
    static constexpr std::size_t pad_size =
        cache_aligned_detail::cache_line_size - (sizeof(T) % cache_aligned_detail::cache_line_size);
    char pad[pad_size == cache_aligned_detail::cache_line_size ? 0 : pad_size];
};
