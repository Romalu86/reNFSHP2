#pragma once
#include <cstdint>
#include <exception>

namespace nfs::lifecycle::construction {
namespace array_detail {
template<class Element>
void destroy_remaining(std::uintptr_t next, int stride, int count,
                       void (*destroy)(Element *)) {
    const auto step = static_cast<std::uintptr_t>(stride);
    try {
        for (;;) {
            count = static_cast<int>(std::uint32_t(count) - 1u);
            if (count < 0)
                break;
            next -= step;
            destroy(reinterpret_cast<Element *>(next));
        }
    } catch (...) {
        std::terminate();
    }
}
} // namespace array_detail Typed form of the constructor iterator and reverse-unwind cleanup.
// Constructor results are discarded; callbacks retain their exact C++ types.
 
template<class Element, class Result>
void construct_array(void *storage, int stride, int count,
                     Result (*construct)(Element *), void (*destroy)(Element *)) {
    auto next = reinterpret_cast<std::uintptr_t>(storage);
    const auto step = static_cast<std::uintptr_t>(stride);
    int completed = 0;
    try {
        for (; completed < count; ++completed) {
            construct(reinterpret_cast<Element *>(next));
            next += step;
        }
    } catch (...) {
        array_detail::destroy_remaining(next, stride, completed, destroy);
        throw;
    }
}
namespace objects {
template<class Element>
void destroy_array(void *storage, int stride, int count, void (*destroy)(Element *)) {
    const auto step = static_cast<std::uintptr_t>(stride);
    auto next = reinterpret_cast<std::uintptr_t>(storage) + step * std::uint32_t(count);
    try {
        for (;;) {
            count = static_cast<int>(std::uint32_t(count) - 1u);
            if (count < 0)
                break;
            next -= step;
            destroy(reinterpret_cast<Element *>(next));
        }
    } catch (...) {
        array_detail::destroy_remaining(next, stride, count, destroy);
        throw;
    }
}
} // namespace objects
} // namespace nfs::lifecycle::construction
