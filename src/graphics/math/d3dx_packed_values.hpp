#pragma once
#include <mm3dnow.h>
#include <cstring>
namespace nfs::graphics::d3dx::packed {
inline __m64 load(const float *values) {
    __m64 result;
    std::memcpy(&result, values, 8);
    return result;
}
inline __m64 low(const float *value) {
    int bits;
    std::memcpy(&bits, value, 4);
    return _m_from_int(bits);
}
inline __m64 repeat_low(__m64 value) {
    return _m_punpckldq(value, value);
}
inline __m64 repeat_high(__m64 value) {
    return _m_punpckhdq(value, value);
}
inline __m64 splat(const float *value) {
    return repeat_low(low(value));
}
inline void store(float *out, __m64 value) {
    std::memcpy(out, &value, 8);
}
inline void store_low(float *out, __m64 value) {
    int bits = _m_to_int(value);
    std::memcpy(out, &bits, 4);
}
inline __m64 bits(int high, int low) {
    unsigned words[2] = {unsigned(low), unsigned(high)};
    __m64 result;
    std::memcpy(&result, words, 8);
    return result;
}
inline __m64 reciprocal(__m64 value) {
    auto estimate = _m_pfrcp(value);
    return _m_pfrcpit2(_m_pfrcpit1(value, estimate), estimate);
}
inline __m64 inverse_root(__m64 square) {
    auto estimate = _m_pfrsqrt(square);
    auto correction = _m_pfrsqit1(square, _m_pfmul(estimate, estimate));
    return _m_pfrcpit2(correction, estimate);
}
} // namespace nfs::graphics::d3dx::packed
