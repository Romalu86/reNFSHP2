#include "d3dx_packed_matrix_values.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;
namespace {
__m64 squared_distance_pairs(const float *a, const float *b, bool sum) {
    auto xy = sum ? _m_pfadd(load(a), load(b)) : _m_pfsub(load(a), load(b));
    auto zw = sum ? _m_pfadd(load(a + 2), load(b + 2)) : _m_pfsub(load(a + 2), load(b + 2));
    return _m_pfadd(_m_pfmul(xy, xy), _m_pfmul(zw, zw));
}
void negate_rotation(float *rotation) {
    auto mask = bits(int(2147483648u), int(2147483648u));
    store(rotation, _m_pxor(load(rotation), mask));
    store(rotation + 2, _m_pxor(load(rotation + 2), mask));
}
void squad_control(float *out, const float *previous, const float *center, const float *next) {
    float inverse[4], before[4], after[4], logarithm_before[4], logarithm_after[4], exponent[4];
    inverse_quaternion_values(inverse, center);
    multiply_quaternion_values(before, previous, inverse);
    multiply_quaternion_values(after, next, inverse);
    logarithm_quaternion_values(logarithm_before, before);
    logarithm_quaternion_values(logarithm_after, after);
    auto quarter = bits(int(3196059648u), int(3196059648u));
    store(exponent, _m_pfmul(_m_pfadd(load(logarithm_before), load(logarithm_after)), quarter));
    store(exponent + 2,
          _m_pfmul(_m_pfadd(load(logarithm_before + 2), load(logarithm_after + 2)), quarter));
    exponential_quaternion_values(exponent, exponent);
    multiply_quaternion_values(out, exponent, center);
}
} // namespace

void __stdcall setup_squad_packed(float *first_control, float *second_control, float *end,
                                  const float *previous, const float *first, const float *second,
                                  const float *following) {
    float adjusted_previous[4], saved_first[4], adjusted_second[4], adjusted_following[4];
    std::memcpy(adjusted_previous, previous, 16);
    std::memcpy(saved_first, first, 16);
    std::memcpy(adjusted_second, second, 16);
    auto plus = _m_pfacc(squared_distance_pairs(adjusted_previous, saved_first, true),
                         squared_distance_pairs(saved_first, adjusted_second, true));
    auto minus = _m_pfacc(squared_distance_pairs(adjusted_previous, saved_first, false),
                          squared_distance_pairs(saved_first, adjusted_second, false));
    auto choices = _m_packsswb(_m_pfcmpgt(minus, plus), _m_pfcmpgt(minus, plus));
    Dword choice = Dword(_m_to_int(choices));
    if (choice & 1)
        negate_rotation(adjusted_previous);
    if (choice & 65536)
        negate_rotation(adjusted_second);
    std::memcpy(adjusted_following, following, 16);
    plus = squared_distance_pairs(adjusted_second, adjusted_following, true);
    minus = squared_distance_pairs(adjusted_second, adjusted_following, false);
    if (Dword(_m_to_int(_m_pfcmpgt(_m_pfacc(minus, minus), _m_pfacc(plus, plus)))) & 1)
        negate_rotation(adjusted_following);
    squad_control(first_control, adjusted_previous, saved_first, adjusted_second);
    squad_control(second_control, saved_first, adjusted_second, adjusted_following);
    store(end, load(adjusted_second));
    store(end + 2, load(adjusted_second + 2));
    _m_femms();
}
} // namespace nfs::graphics::d3dx
