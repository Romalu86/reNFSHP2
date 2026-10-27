#include "d3dx_trigonometric_constants.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx::trigonometry {
namespace {
__m64 coefficient(const Dword *values) {
    __m64 result;
    std::memcpy(&result, values, 8);
    return result;
}
} // namespace
// Saved MM0 input/result have an explicit C++ value adapter.
__m64 approximate_sine_cosine_packed(__m64 input) {
    Dword input_bits = Dword(_m_to_int(input));
    Dword sign = input_bits & 2147483648u, absolute_bits = input_bits ^ sign;
    auto absolute = _m_pand(input, _m_from_int(int(packed_absolute_mask)));
    if (int(absolute_bits) >= int(packed_argument_limit))
        return _m_from_int(int(packed_invalid_result));
    auto scaled = _m_pfmul(absolute, _m_from_int(int(packed_two_over_pi)));
    auto integer = _m_pf2id(scaled);
    Dword quadrant = Dword(_m_to_int(integer));
    auto reduction = _m_pfmul(packed::repeat_low(_m_pi2fd(integer)), coefficient(packed_reduction));
    auto reduced = _m_pfadd(absolute, reduction);
    reduced = _m_pfadd(reduced, packed::repeat_high(reduction));
    reduced = packed::repeat_low(reduced);
    if (quadrant & 1)
        reduced = _m_pfsubr(reduced, coefficient(packed_half_pi));
    Dword shared_sign = ((input_bits >> 31) ^ (quadrant >> 2)) << 31;
    Dword low_sign = sign ^ ((quadrant >> 1) << 31);
    auto unit = coefficient(packed_unit);
    auto factor = _m_punpckldq(unit, reduced);
    auto square = _m_pfmul(reduced, reduced);
    auto a = _m_pfmul(coefficient(packed_polynomial_a), square);
    auto b = _m_pfmul(coefficient(packed_polynomial_b), square);
    auto c = _m_pfmul(coefficient(packed_polynomial_c), square);
    auto d = _m_pfmul(coefficient(packed_polynomial_d), square);
    auto polynomial = _m_pfadd(_m_pfmul(_m_pfadd(a, unit), b), unit);
    polynomial = _m_pfadd(_m_pfmul(c, polynomial), unit);
    polynomial = _m_pfmul(polynomial, _m_punpckldq(d, unit));
    auto unit_high = _m_psrlqi(unit, 32);
    polynomial = _m_pfadd(polynomial, unit_high);
    auto result = _m_pfmul(polynomial, factor);
    if ((quadrant ^ (quadrant >> 1)) & 1)
        result = _m_punpckldq(packed::repeat_high(result), result);
    result = _m_pxor(result, _m_from_int(int(low_sign)));
    return _m_pxor(packed::bits(int(shared_sign), int(shared_sign)), result);
}
// Sine entry returns its high lane in both words.
__m64 approximate_sine_packed(__m64 input) {
    Dword input_bits = Dword(_m_to_int(input));
    Dword sign = input_bits & 2147483648u, absolute_bits = input_bits ^ sign;
    auto absolute = _m_pand(input, _m_from_int(int(packed_absolute_mask)));
    if (int(absolute_bits) >= int(packed_argument_limit))
        return _m_from_int(0);
    auto scaled = _m_pfmul(absolute, _m_from_int(int(packed_two_over_pi)));
    auto integer = _m_pf2id(scaled);
    Dword quadrant = Dword(_m_to_int(integer));
    auto reduction = _m_pfmul(packed::repeat_low(_m_pi2fd(integer)), coefficient(packed_reduction));
    auto reduced = _m_pfadd(absolute, reduction);
    reduced = _m_pfadd(reduced, packed::repeat_high(reduction));
    reduced = packed::repeat_low(reduced);
    if (quadrant & 1)
        reduced = _m_pfsubr(reduced, coefficient(packed_half_pi));
    Dword shared_sign = ((input_bits >> 31) ^ (quadrant >> 2)) << 31;
    Dword low_sign = sign ^ ((quadrant >> 1) << 31);
    auto unit = coefficient(packed_unit);
    auto factor = _m_punpckldq(unit, reduced);
    auto square = _m_pfmul(reduced, reduced);
    auto a = _m_pfmul(coefficient(packed_polynomial_a), square);
    auto b = _m_pfmul(coefficient(packed_polynomial_b), square);
    auto c = _m_pfmul(coefficient(packed_polynomial_c), square);
    auto d = _m_pfmul(coefficient(packed_polynomial_d), square);
    auto polynomial = _m_pfadd(_m_pfmul(_m_pfadd(a, unit), b), unit);
    polynomial = _m_pfadd(_m_pfmul(c, polynomial), unit);
    polynomial = _m_pfmul(polynomial, _m_punpckldq(d, unit));
    auto unit_high = _m_psrlqi(unit, 32);
    polynomial = _m_pfadd(polynomial, unit_high);
    auto result = _m_pfmul(polynomial, factor);
    if ((quadrant ^ (quadrant >> 1)) & 1)
        result = _m_punpckldq(packed::repeat_high(result), result);
    result = _m_pxor(result, _m_from_int(int(low_sign)));
    return packed::repeat_high(_m_pxor(packed::bits(int(shared_sign), int(shared_sign)), result));
}
} // namespace nfs::graphics::d3dx::trigonometry
