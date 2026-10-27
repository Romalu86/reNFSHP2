#include "d3dx_packed_matrix_values.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;
namespace {
__m64 swap_words(__m64 value) {
    return _m_punpckldq(repeat_high(value), value);
}
} // namespace
void packed::multiply_quaternion_values(float *out, const float *first, const float *second) {
    auto bxy = load(second), bzw = load(second + 2);
    auto axy = load(first), azw = load(first + 2);
    auto swapped_xy = swap_words(bxy), swapped_zw = swap_words(bzw);
    auto sign_x = bits(0, int(2147483648u));
    auto cross_xy = _m_pfadd(_m_pxor(_m_pfmul(bxy, azw), sign_x), _m_pfmul(bzw, axy));
    auto cross_yx = _m_pfsub(_m_pfmul(swapped_xy, azw), _m_pfmul(_m_pxor(swapped_zw, sign_x), axy));
    auto xy = _m_pfacc(cross_yx, cross_xy);
    auto cross_z = _m_pfadd(_m_pfmul(_m_pxor(swapped_xy, sign_x), axy), _m_pfmul(swapped_zw, azw));
    // These loads precede the first output store, including in-place calls.
    auto dot_xy = _m_pfmul(load(second), axy);
    auto signed_zw = _m_pxor(load(second + 2), sign_x);
    store(out, xy);
    auto dot_zw = _m_pfsub(_m_pfmul(signed_zw, azw), dot_xy);
    store(out + 2, _m_pfacc(cross_z, dot_zw));
}

float *__stdcall multiply_quaternion_packed(float *out, const float *first, const float *second) {
    multiply_quaternion_values(out, first, second);
    _m_femms();
    return out;
}
void packed::inverse_quaternion_values(float *out, const float *input) {
    auto xy = load(input), zw = load(input + 2);
    auto square = _m_pfadd(_m_pfmul(xy, xy), _m_pfmul(zw, zw));
    square = _m_pfacc(square, square);
    auto estimate = _m_pfrcp(square);
    auto mask = _m_pfcmpgt(square, bits(8388608, 8388608));
    xy = _m_pand(_m_pxor(xy, bits(int(2147483648u), int(2147483648u))), mask);
    zw = _m_pand(_m_pxor(zw, bits(0, int(2147483648u))), mask);
    auto inverse = _m_pfrcpit2(_m_pfrcpit1(square, estimate), estimate);
    store(out, _m_pfmul(xy, inverse));
    store(out + 2, _m_pfmul(zw, inverse));
}

float *__stdcall inverse_quaternion_packed(float *out, const float *input) {
    inverse_quaternion_values(out, input);
    _m_femms();
    return out;
}
void packed::quaternion_matrix_values(float *out, const float *rotation) {
    auto xy = load(rotation), zw = load(rotation + 2);
    auto twice_xy = _m_pfadd(xy, xy), twice_zw = _m_pfadd(zw, zw);
    auto w = repeat_high(zw), y = repeat_high(xy);
    auto squares = _m_pfmul(xy, twice_xy);
    auto twice_zx = _m_punpckldq(twice_zw, twice_xy);
    auto xyw = _m_pfmul(twice_xy, w);
    auto zxz = _m_pfmul(repeat_low(zw), twice_zx);
    auto zxy = _m_pfmul(twice_zx, y), zxw = _m_pfmul(twice_zx, w);
    auto off_diagonal = _m_punpckhdq(zxy, zxz);
    auto diagonal_sum = _m_pfadd(repeat_low(zxz), squares);
    auto z_diagonal = _m_pfsubr(_m_pfacc(squares, squares), bits(1065353216, 1065353216));
    auto diagonal = _m_pfsubr(diagonal_sum, bits(1065353216, 1065353216));
    auto cross_w = _m_punpckldq(zxw, repeat_high(xyw));
    store(out + 12, _m_from_int(0));
    auto positive = _m_pfadd(off_diagonal, cross_w), negative = _m_pfsub(off_diagonal, cross_w);
    store(out + 10, _m_pand(z_diagonal, bits(0, -1)));
    auto yz_plus = _m_pfadd(zxy, xyw), yz_minus = _m_pfsub(zxy, xyw);
    store(out + 6, _m_pand(yz_plus, bits(0, -1)));
    store(out + 2, _m_psrlqi(negative, 32));
    store(out + 4, _m_punpckldq(negative, diagonal));
    store(out, _m_punpckldq(repeat_high(diagonal), positive));
    store(out + 8, _m_punpckldq(repeat_high(positive), yz_minus));
    store(out + 14, bits(1065353216, 0));
}

void __stdcall quaternion_matrix_packed(float *out, const float *rotation) {
    quaternion_matrix_values(out, rotation);
    _m_femms();
}
void packed::quaternion_matrix_extended_values(float *out, const float *rotation) {
    auto xy = load(rotation), zw = load(rotation + 2);
    auto twice_xy = _m_pfadd(xy, xy), twice_zw = _m_pfadd(zw, zw);
    auto squares = _m_pfmul(twice_xy, xy);
    auto xz_yw = _m_pfmul(xy, twice_zw);
    auto yz_xw = _m_pfmul(_m_pswapd(xy), twice_zw);
    auto z_square = _m_pfmul(twice_zw, zw);
    auto y_w = _m_punpckhdq(xy, zw);
    auto twice_x_z = _m_punpckldq(twice_xy, twice_zw);
    auto xy_zw = _m_pfmul(y_w, twice_x_z);
    auto diagonal =
        _m_pfsub(_m_pfsubr(squares, bits(1065353216, 1065353216)), repeat_low(z_square));
    auto yz_xw_sum = _m_pfpnacc(yz_xw, yz_xw);
    auto z_diagonal = _m_pfsubr(_m_pfacc(squares, squares), bits(1065353216, 1065353216));
    auto xy_zw_sum = _m_pfpnacc(xy_zw, xy_zw), xz_yw_sum = _m_pfpnacc(xz_yw, xz_yw);
    store(out + 12, _m_from_int(0));
    store(out + 8, _m_punpckldq(repeat_high(xz_yw_sum), yz_xw_sum));
    store(out + 2, _m_pand(xz_yw_sum, bits(0, -1)));
    store(out + 10, _m_psrlqi(z_diagonal, 32));
    store(out, _m_punpckhdq(diagonal, xy_zw_sum));
    store(out + 4, _m_punpckldq(xy_zw_sum, diagonal));
    store(out + 6, _m_psrlqi(yz_xw_sum, 32));
    store(out + 14, bits(1065353216, 0));
}

void __stdcall quaternion_matrix_packed_extended(float *out, const float *rotation) {
    quaternion_matrix_extended_values(out, rotation);
    _m_femms();
}
} // namespace nfs::graphics::d3dx
