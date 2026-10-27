#include "d3dx_dispatch_kernels.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;

void __stdcall plane_from_points_packed(float *out, const float *first, const float *second,
                                        const float *third) {
    _m_femms();
    auto first_z = low(first + 2);
    auto a = _m_pfsub(load(first), load(second));
    auto b = _m_pfsub(load(first), load(third));
    auto negative_b = _m_pfsub(_m_from_int(0), b);
    auto z_differences =
        _m_pfsub(repeat_low(first_z), _m_punpckldq(low(third + 2), low(second + 2)));
    auto first_products = _m_pfmul(_m_punpckhdq(a, negative_b), z_differences);
    auto second_products = _m_pfmul(_m_punpckldq(repeat_high(z_differences), a),
                                    _m_punpckldq(negative_b, z_differences));
    auto xy = _m_pxor(_m_pfacc(first_products, second_products), bits(int(2147483648u), 0));
    auto z_products = _m_pfmul(_m_punpckldq(repeat_high(b), negative_b), a);
    auto z = _m_pfacc(z_products, z_products);
    auto square = _m_pfadd(_m_pfmul(xy, xy), _m_pfmul(_m_psllqi(z, 32), _m_psllqi(z, 32)));
    square = _m_pfacc(square, square);
    auto estimate = _m_pfrsqrt(square);
    auto mask = _m_pfcmpgt(square, bits(8388608, 8388608));
    auto inverse = _m_pfrcpit2(_m_pfrsqit1(square, _m_pfmul(estimate, estimate)), estimate);
    xy = _m_pfmul(_m_pand(xy, mask), inverse);
    z = _m_pfmul(_m_pand(z, mask), inverse);
    auto dot = _m_pfmul(xy, load(first));
    dot = _m_pfadd(_m_pfacc(dot, dot), _m_pfmul(z, first_z));
    store(out, xy);
    store(out + 2, _m_pxor(_m_punpckldq(z, dot), bits(int(2147483648u), 0)));
    _m_femms();
}

float *__stdcall intersect_plane_line_packed(float *out, const float *plane, const float *first,
                                             const float *second) {
    auto normal_xy = load(plane), normal_zw = load(plane + 2), first_xy = load(first);
    auto second_dot =
        _m_pfadd(_m_pfmul(normal_xy, load(second)), _m_pfmul(normal_zw, low(second + 2)));
    auto first_dot = _m_pfadd(_m_pfmul(normal_xy, first_xy), _m_pfmul(normal_zw, low(first + 2)));
    auto difference = _m_pfsub(first_dot, second_dot);
    auto first_z_one = _m_punpckldq(low(first + 2), bits(0, 1065353216));
    auto denominator = _m_pfacc(difference, difference);
    auto equal = _m_pfcmpeq(_m_from_int(0), denominator);
    auto estimate = _m_pfrcp(denominator);
    if (_m_to_int(equal)) {
        _m_femms();
        return nullptr;
    }
    auto numerator = _m_pfadd(_m_pfmul(normal_xy, first_xy), _m_pfmul(normal_zw, first_z_one));
    auto inverse = _m_pfrcpit2(_m_pfrcpit1(denominator, estimate), estimate);
    auto fraction = _m_pfmul(_m_pfacc(numerator, numerator), inverse);
    auto xy = _m_pfadd(_m_pfmul(_m_pfsub(load(second), first_xy), fraction), first_xy);
    auto z = _m_pfadd(_m_pfmul(_m_pfsub(low(second + 2), first_z_one), fraction), first_z_one);
    store(out, xy);
    store_low(out + 2, z);
    _m_femms();
    return out;
}
} // namespace nfs::graphics::d3dx
