#include "d3dx_dispatch_kernels.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;

void __stdcall shadow_matrix_packed(float *out, const float *light, const float *plane) {
    _m_femms();
    float normal[4];
    normalize_plane_packed(normal, plane);
    auto light_xy = load(light), light_zw = load(light + 2);
    auto product = _m_pfadd(_m_pfmul(light_xy, load(normal)), _m_pfmul(light_zw, load(normal + 2)));
    auto dot = _m_pfacc(product, product);
    auto sign = bits(int(2147483648u), int(2147483648u));
    auto negative_xy = _m_pxor(load(normal), sign), negative_zw = _m_pxor(load(normal + 2), sign);
    auto row = repeat_low(negative_xy);
    store(out, _m_pfadd(_m_pfmul(row, light_xy), _m_pand(dot, bits(0, -1))));
    store(out + 2, _m_pfmul(row, light_zw));
    row = repeat_high(negative_xy);
    store(out + 4, _m_pfadd(_m_pfmul(row, light_xy), _m_psllqi(dot, 32)));
    store(out + 6, _m_pfmul(row, light_zw));
    row = repeat_low(negative_zw);
    store(out + 8, _m_pfmul(row, light_xy));
    store(out + 10, _m_pfadd(_m_pfmul(row, light_zw), _m_pand(dot, bits(0, -1))));
    row = repeat_high(negative_zw);
    store(out + 12, _m_pfmul(row, light_xy));
    store(out + 14, _m_pfadd(_m_pfmul(row, light_zw), _m_psllqi(dot, 32)));
    _m_femms();
}

void __stdcall reflection_matrix_packed(float *out, const float *plane) {
    _m_femms();
    float normal[4];
    normalize_plane_packed(normal, plane);
    auto xy = load(normal), zw = load(normal + 2);
    auto scaled_xy = _m_pfmul(xy, bits(int(3221225472u), int(3221225472u)));
    auto scaled_zw = _m_pfmul(zw, bits(0, int(3221225472u)));
    store(out, _m_pfadd(_m_pfmul(repeat_low(xy), scaled_xy), bits(0, 1065353216)));
    store(out + 2, _m_pfmul(_m_pand(xy, bits(0, -1)), scaled_zw));
    store(out + 4, _m_pfadd(_m_pfmul(repeat_high(xy), scaled_xy), bits(1065353216, 0)));
    store(out + 6, _m_pfmul(_m_psrlqi(xy, 32), scaled_zw));
    store(out + 8, _m_pfmul(repeat_low(zw), scaled_xy));
    store(out + 10, _m_pfadd(_m_pfmul(_m_pand(zw, bits(0, -1)), scaled_zw), bits(0, 1065353216)));
    store(out + 12, _m_pfmul(repeat_high(zw), scaled_xy));
    store(out + 14, _m_pfadd(_m_pfmul(_m_psrlqi(zw, 32), scaled_zw), bits(1065353216, 0)));
    _m_femms();
}
} // namespace nfs::graphics::d3dx
