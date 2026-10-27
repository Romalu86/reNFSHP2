#include "d3dx_packed_matrix_values.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;
namespace {
struct CofactorPairs {
    __m64 first, second, third, fourth;
};
CofactorPairs cofactor_pairs(const float *m, unsigned columns, bool enhanced) {
    auto row0 = load(m + columns), row1 = load(m + 4 + columns);
    auto row2 = load(m + 8 + columns), row3 = load(m + 12 + columns);
    __m64 a, b, c;
    if (enhanced) {
        a = _m_pfnacc(_m_pfmul(_m_pswapd(row1), row0), _m_pfmul(_m_pswapd(row2), row0));
        b = _m_pfnacc(_m_pfmul(_m_pswapd(row3), row0), _m_pfmul(_m_pswapd(row2), row1));
        c = _m_pfnacc(_m_pfmul(row1, _m_pswapd(row3)), _m_pfmul(row2, _m_pswapd(row3)));
    } else {
        a = _m_pfsub(_m_pfmul(repeat_low(row0), _m_punpckhdq(row1, row2)),
                     _m_pfmul(repeat_high(row0), _m_punpckldq(row1, row2)));
        b = _m_pfsub(_m_pfmul(_m_punpckhdq(row3, row2), _m_punpckldq(row0, row1)),
                     _m_pfmul(_m_punpckhdq(row0, row1), _m_punpckldq(row3, row2)));
        c = _m_pfsub(_m_pfmul(_m_punpckldq(row1, row2), repeat_high(row3)),
                     _m_pfmul(_m_punpckhdq(row1, row2), repeat_low(row3)));
    }
    unsigned companion = columns ^ 2;
    auto last = load(m + 12 + companion), third = load(m + 8 + companion);
    auto second = load(m + 4 + companion), first = load(m + companion);
    CofactorPairs result;
    result.first =
        _m_pfsub(_m_pfadd(_m_pfmul(repeat_high(b), last), _m_pfmul(repeat_high(c), second)),
                 _m_pfmul(repeat_low(c), third));
    result.second =
        _m_pfadd(_m_pfsub(_m_pfmul(repeat_high(a), last), _m_pfmul(repeat_low(b), third)),
                 _m_pfmul(repeat_high(c), first));
    result.third = _m_pfsub(_m_pfadd(_m_pfmul(last, repeat_low(a)), _m_pfmul(repeat_low(c), first)),
                            _m_pfmul(repeat_low(b), second));
    result.fourth =
        _m_pfadd(_m_pfsub(_m_pfmul(repeat_low(a), third), _m_pfmul(repeat_high(a), second)),
                 _m_pfmul(repeat_high(b), first));
    return result;
}
float *inverse_pairs(float *out, float *determinant, const float *m, bool enhanced) {
    auto upper = cofactor_pairs(m, 0, enhanced), lower = cofactor_pairs(m, 2, enhanced);
    auto high_lower = _m_punpckhdq(lower.fourth, lower.third);
    auto high_upper = _m_punpckhdq(lower.first, lower.second);
    auto m02 = _m_punpckldq(low(m), low(m + 4)), m13 = _m_punpckldq(low(m + 8), low(m + 12));
    auto signs = bits(int(3212836864u), 1065353216);
    __m64 value;
    if (enhanced)
        value = _m_pfnacc(_m_pfmul(high_lower, m13), _m_pfmul(m02, high_upper));
    else
        value = _m_pfadd(_m_pfmul(high_lower, _m_pfmul(m13, signs)),
                         _m_pfmul(_m_pfmul(m02, signs), high_upper));
    value = _m_pfacc(value, value);
    if (determinant)
        store_low(determinant, value);
    auto estimate = _m_pfrcp(value);
    auto zero_mask = _m_pfcmpeq(_m_from_int(0), value);
    auto inverse = _m_pfrcpit2(_m_pfrcpit1(repeat_low(value), estimate), estimate);
    auto signed_inverse = _m_pfmul(inverse, signs);
    auto reversed_inverse = enhanced ? _m_pswapd(signed_inverse)
                                     : _m_punpckldq(repeat_high(signed_inverse), signed_inverse);
    Dword failure = Dword(_m_to_int(zero_mask));
    if (failure)
        return reinterpret_cast<float *>(failure); // A zero determinant returns all bits set.
    auto first = _m_pfmul(lower.first, reversed_inverse),
         second = _m_pfmul(lower.second, signed_inverse);
    auto third = _m_pfmul(upper.first, reversed_inverse),
         fourth = _m_pfmul(upper.second, signed_inverse);
    store(out, _m_punpckhdq(first, second));
    store(out + 4, _m_punpckldq(first, second));
    store(out + 8, _m_punpckhdq(third, fourth));
    store(out + 12, _m_punpckldq(third, fourth));
    first = _m_pfmul(lower.third, reversed_inverse);
    second = _m_pfmul(lower.fourth, signed_inverse);
    third = _m_pfmul(upper.third, reversed_inverse);
    fourth = _m_pfmul(upper.fourth, signed_inverse);
    store(out + 2, _m_punpckhdq(first, second));
    store(out + 6, _m_punpckldq(first, second));
    store(out + 10, _m_punpckhdq(third, fourth));
    store(out + 14, _m_punpckldq(third, fourth));
    return out;
}
} // namespace
namespace packed {

float *__stdcall invert_cofactor_pairs(float *out, float *determinant, const float *matrix) {
    return inverse_pairs(out, determinant, matrix, false);
}
} // namespace packed

float *__stdcall invert_matrix_packed(float *out, float *determinant, const float *matrix) {
    _m_femms();
    invert_cofactor_pairs(out, determinant, matrix);
    _m_femms();
    return out; // The saved public wrapper discards its helper's failure word.
}

float *__stdcall invert_matrix_packed_extended(float *out, float *determinant,
                                               const float *matrix) {
    auto result = inverse_pairs(out, determinant, matrix, true);
    _m_femms();
    return result;
}
} // namespace nfs::graphics::d3dx
