#include "d3dx_trigonometric_constants.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx::trigonometry {
using namespace packed;
namespace {
__m64 scalar(Dword word) {
    return _m_from_int(int(word));
}
} // namespace
// MM0/MM1 register inputs are explicit C++ values.
__m64 approximate_atan2_packed(__m64 x, __m64 y) {
    auto sign = scalar(2147483648u);
    auto combined_sign = _m_por(_m_psrldi(_m_pand(sign, x), 1), _m_pand(sign, y));
    auto absolute_x = _m_pand(x, scalar(2147483647u));
    auto absolute_y = _m_pand(y, scalar(2147483647u));
    auto exchanged = _m_pslldi(_m_pcmpgtd(absolute_y, absolute_x), 31);
    auto maximum = _m_pfmax(absolute_x, absolute_y);
    auto minimum = _m_pfmin(absolute_y, absolute_x);
    auto ratio = _m_pfmul(reciprocal(maximum), minimum);
    auto square = _m_pfmul(ratio, ratio);
    auto denominator = _m_pfadd(scalar(1094016448u), square);
    auto numerator = _m_pfmul(scalar(3209873386u), square);
    denominator = _m_pfmul(denominator, square);
    numerator = _m_pfadd(numerator, scalar(3233126531u));
    denominator = _m_pfadd(denominator, scalar(1105640800u));
    numerator = _m_pfmul(numerator, square);
    denominator = _m_pfmul(denominator, square);
    numerator = _m_pfadd(numerator, scalar(3234994439u));
    denominator = _m_pfadd(denominator, scalar(1100845768u));
    auto correction = _m_pfmul(_m_pfmul(square, numerator), ratio);
    auto doubled_sign = _m_pslldi(combined_sign, 1);
    correction = _m_pfmul(correction, reciprocal(denominator));
    auto half_pi = scalar(3217625051u);
    auto offset = _m_pandn(_m_psradi(exchanged, 31), _m_pxor(doubled_sign, half_pi));
    offset = _m_pfsub(offset, half_pi);
    auto angle = _m_por(_m_pfadd(correction, ratio), _m_pxor(doubled_sign, exchanged));
    return _m_por(_m_pfadd(angle, offset), _m_pand(sign, combined_sign));
}
// The input magnitude tests compare saved integer words.
__m64 approximate_acos_packed(__m64 input) {
    auto sign = _m_pand(scalar(2147483648u), input);
    auto absolute = _m_pand(input, scalar(2147483647u));
    auto invalid = _m_pcmpgtd(absolute, scalar(1065353216u));
    auto half = scalar(1056964608u);
    auto square = _m_pfsubr(_m_pfmul(absolute, half), half);
    auto estimate = _m_pfrsqrt(square);
    auto root =
        _m_pfmul(_m_pfrcpit2(_m_pfrsqit1(_m_pfmul(estimate, estimate), square), estimate), square);
    auto small = _m_pcmpgtd(scalar(1058222899u), absolute);
    auto reduced = _m_por(_m_pandn(small, root), _m_pand(absolute, small));
    square = _m_pfmul(reduced, reduced);
    auto numerator = _m_pfmul(scalar(3204467756u), square);
    auto denominator = _m_pfadd(scalar(3232715043u), square);
    numerator = _m_pfadd(numerator, scalar(1063930533u));
    numerator = _m_pfmul(numerator, square);
    denominator = _m_pfmul(square, denominator);
    denominator = _m_pfadd(denominator, scalar(1085257769u));
    numerator = _m_pfmul(numerator, reduced);
    auto correction = _m_pfmul(reciprocal(denominator), numerator);
    auto offset = _m_pxor(_m_pandn(small, scalar(3217625051u)), sign);
    offset = _m_pfadd(offset, scalar(1070141403u));
    auto angle = _m_pfadd(reduced, correction);
    auto result_sign = _m_pxor(_m_pslldi(small, 31), sign);
    angle = _m_pfadd(angle, _m_pandn(small, angle));
    angle = _m_pfadd(offset, _m_por(angle, result_sign));

    return _m_pandn(invalid, angle);
}
} // namespace nfs::graphics::d3dx::trigonometry
