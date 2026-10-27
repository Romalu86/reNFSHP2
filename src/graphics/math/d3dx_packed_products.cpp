#include "d3dx_dispatch_kernels.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;

float __stdcall matrix_determinant_packed(const float *m) {
    auto a = _m_pfsub(_m_pfmul(splat(m + 15), load(m + 9)), _m_pfmul(splat(m + 11), load(m + 13)));
    auto b = _m_pfsub(_m_pfmul(splat(m + 8), load(m + 14)), _m_pfmul(splat(m + 12), load(m + 10)));
    auto c = _m_pfsub(_m_pfmul(load(m + 13), load(m + 8)), _m_pfmul(load(m + 9), load(m + 12)));
    auto first = _m_pfadd(_m_pfmul(a, load(m + 4)), _m_pfmul(c, splat(m + 7)));
    first = _m_pfsub(first, _m_pfmul(_m_punpckldq(repeat_high(b), a), load(m + 5)));
    auto left = _m_pfmul(first, _m_punpckldq(low(m + 2), low(m)));
    auto second = _m_pfadd(_m_pfmul(_m_punpckhdq(c, a), splat(m + 4)),
                           _m_pfmul(_m_punpckldq(c, b), load(m + 6)));
    second = _m_pfsub(second, _m_pfmul(b, load(m + 5)));
    auto right = _m_pfmul(second, _m_punpckldq(low(m + 3), low(m + 1)));
    auto difference = _m_pfsub(left, right);
    float result;
    store_low(&result, _m_pfacc(difference, difference));
    _m_femms();
    return result;
}

void __stdcall cross_vector4_packed(float *out, const float *a, const float *b, const float *c) {
    _m_femms();
    auto sign_high = bits(int(2147483648u), 0), sign_low = bits(0, int(2147483648u));
    auto bzy = _m_punpckldq(low(b + 2), low(b + 1));
    auto czy = _m_punpckldq(low(c + 2), low(c + 1));
    auto zw_yw = _m_pfsub(_m_pfmul(splat(c + 3), bzy), _m_pfmul(splat(b + 3), czy));
    auto zy_wx =
        _m_pfsub(_m_pfmul(_m_punpckldq(low(b + 1), low(b + 3)), _m_punpckldq(low(c + 2), low(c))),
                 _m_pfmul(_m_punpckldq(low(c + 1), low(c + 3)), _m_punpckldq(low(b + 2), low(b))));
    auto zx_yx = _m_pfsub(_m_pfmul(splat(c), bzy), _m_pfmul(splat(b), czy));
    auto axy = load(a), azw = load(a + 2);
    auto swapped_xy = _m_punpckldq(repeat_high(axy), axy);
    auto lead = _m_pfmul(_m_pxor(repeat_low(zw_yw), sign_high), swapped_xy);
    auto middle = _m_pfmul(_m_punpckldq(repeat_high(zw_yw), zy_wx), azw);
    auto tail = _m_pfmul(_m_punpckldq(repeat_high(zy_wx), zx_yx), azw);
    auto xy = _m_pfsub(lead, _m_pfacc(_m_pxor(middle, sign_high), _m_pxor(tail, sign_high)));
    auto first = _m_pfmul(_m_pxor(_m_punpckhdq(zw_yw, zy_wx), sign_high), axy);
    auto second = _m_pfmul(_m_pxor(_m_punpckldq(zy_wx, zx_yx), sign_low), axy);
    auto swapped_zw = _m_punpckldq(repeat_high(azw), azw);
    auto third = _m_pfmul(swapped_zw, _m_pxor(repeat_high(zx_yx), sign_low));
    auto zw = _m_pfadd(third, _m_pfacc(_m_pxor(first, sign_high), _m_pxor(second, sign_high)));
    store(out, xy);
    store(out + 2, zw);
    _m_femms();
}
} // namespace nfs::graphics::d3dx
