#include "d3dx_dispatch_kernels.hpp"
#include "d3dx_packed_values.hpp"
#include <xmmintrin.h>
namespace nfs::graphics::d3dx {
using namespace packed;
namespace {
__m64 half_inverse_root(__m64 input) {
    auto estimate = _m_pfrsqrt(input);
    auto square = _m_pfmul(estimate, estimate);
    auto repeated = repeat_low(input);
    return _m_pfmul(_m_pfrcpit2(_m_pfrsqit1(square, repeated), estimate),
                    bits(1056964608, 1056964608));
}
void matrix_quaternion(float *out, const float *matrix, bool enhanced) {
    auto first = load(matrix), middle = low(matrix + 5), last = low(matrix + 10);
    auto sum = _m_pfadd(_m_pfadd(first, middle), last);
    auto x_case = _m_pfsub(_m_pfsub(first, middle), last);
    auto y_case = _m_pfsub(_m_pfsubr(first, middle), last);
    auto z_case = repeat_low(_m_pfsub(_m_pfsubr(first, last), middle));
    sum = _m_punpckldq(sum, y_case);
    auto first_test = _m_pfcmpge(_m_punpckldq(_m_from_int(0), x_case), sum);
    auto second_test = _m_pfcmpge(_m_punpckldq(y_case, x_case), z_case);
    auto packed_tests = _m_packssdw(first_test, second_test);
    Dword selection = enhanced ? Dword(_mm_movemask_pi8(packed_tests))
                              : Dword(_m_to_int(_m_packsswb(packed_tests, packed_tests)));
    auto zx_zy = load(matrix + 8);
    auto one_minus_one = bits(int(3212836864u), 1065353216);
    auto high_sign = bits(int(2147483648u), 0);
    if (!(selection & 1)) {
        sum = _m_pfadd(sum, one_minus_one);
        auto scale = half_inverse_root(sum);
        auto xy = _m_pfsub(_m_punpckldq(low(matrix + 6), zx_zy),
                           _m_punpckhdq(zx_zy, repeat_low(low(matrix + 2))));
        auto z = _m_pfsub(repeat_high(x_case), low(matrix + 4));
        auto w = _m_pfmul(repeat_low(sum), scale);
        z = _m_pfmul(z, scale);
        xy = _m_pfmul(xy, scale);
        store(out, xy);
        store(out + 2, _m_punpckldq(z, w));
    } else if ((selection & (enhanced ? 204u : 4278255360u)) == (enhanced ? 204u : 4278255360u)) {
        x_case = _m_pfadd(x_case, one_minus_one);
        auto scale = half_inverse_root(x_case);
        auto y = _m_pfsub(low(matrix + 4), repeat_high(y_case));
        auto zw = _m_pfadd(_m_punpckldq(zx_zy, low(matrix + 6)),
                           _m_pxor(_m_punpckldq(low(matrix + 2), repeat_high(zx_zy)), high_sign));
        auto x = _m_pfmul(repeat_low(x_case), scale);
        y = _m_pfmul(y, scale);
        zw = _m_pfmul(zw, scale);
        store(out + 2, zw);
        store(out, _m_punpckldq(x, y));
    } else if (selection & (enhanced ? 16u : 65536u)) {
        y_case = _m_pfadd(y_case, one_minus_one);
        auto scale = half_inverse_root(y_case);
        auto x = _m_pfadd(repeat_high(x_case), low(matrix + 4));
        auto zw = _m_pfadd(_m_punpckldq(low(matrix + 6), zx_zy),
                           _m_pxor(_m_punpckldq(repeat_high(zx_zy), low(matrix + 2)), high_sign));
        x = _m_pfmul(x, scale);
        auto y = _m_pfmul(repeat_low(y_case), scale);
        zw = _m_pfmul(zw, scale);
        store(out + 2, zw);
        store(out, _m_punpckldq(x, y));
    } else {
        z_case = _m_pfadd(z_case, one_minus_one);
        auto scale = half_inverse_root(z_case);
        auto w = _m_pfsub(repeat_high(x_case), low(matrix + 4));
        auto xy = _m_pfadd(_m_punpckldq(zx_zy, low(matrix + 6)),
                           _m_punpckldq(low(matrix + 2), repeat_high(zx_zy)));
        auto z = _m_pfmul(repeat_low(z_case), scale);
        w = _m_pfmul(w, scale);
        xy = _m_pfmul(xy, scale);
        store(out, xy);
        store(out + 2, _m_punpckldq(z, w));
    }
    _m_femms();
}
} // namespace

void __stdcall quaternion_from_matrix_packed(float *out, const float *matrix) {
    matrix_quaternion(out, matrix, false);
}

void __stdcall quaternion_from_matrix_packed_extended(float *out, const float *matrix) {
    matrix_quaternion(out, matrix, true);
}
} // namespace nfs::graphics::d3dx
