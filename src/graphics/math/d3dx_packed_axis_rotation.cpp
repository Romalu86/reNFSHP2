#include "d3dx_dispatch_kernels.hpp"
#include "d3dx_packed_values.hpp"
#include "d3dx_trigonometric_constants.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;

void __stdcall axis_rotation_matrix_packed(float *out, const float *axis, float angle) {
    float normal[3];
    store(normal, load(axis));
    store_low(normal + 2, low(axis + 2));
    normalize_vector3_packed(normal, normal);
    auto pair = trigonometry::approximate_sine_cosine_packed(low(&angle));
    auto complement = repeat_low(_m_pfsubr(pair, bits(1065353216, 1065353216)));
    auto xy = load(normal), z = low(normal + 2);
    auto mixed = _m_pfmul(_m_punpckhdq(xy, repeat_low(z)), xy);
    auto zx_zy = _m_pfmul(repeat_low(z), xy);
    mixed = _m_pfmul(mixed, complement);
    zx_zy = _m_pfmul(zx_zy, complement);
    auto diagonal = _m_pfadd(_m_pfmul(_m_pfmul(xy, xy), complement), repeat_low(pair));
    auto last_diagonal = _m_pfadd(_m_pfmul(_m_pfmul(z, z), complement), repeat_low(pair));
    auto sine = repeat_high(pair);
    auto sine_xy = _m_pfmul(xy, sine), sine_z = _m_pfmul(z, sine);
    auto zero = _m_from_int(0);
    auto sin_zx = _m_punpckldq(sine_z, sine_xy);
    auto sine_y = repeat_high(sine_xy);
    auto minus = _m_pfsub(mixed, sin_zx), plus = _m_pfadd(mixed, sin_zx);
    auto xz_minus = _m_pfsub(zx_zy, sine_y);
    auto xz_plus = _m_pfadd(repeat_low(zx_zy), sine_y);
    store(out + 12, zero);
    store(out + 10, _m_punpckldq(last_diagonal, zero));
    store(out, _m_punpckldq(diagonal, plus));
    store(out + 2, _m_punpckldq(xz_minus, zero));
    store(out + 6, _m_punpckhdq(plus, zero));
    store(out + 14, bits(1065353216, 0));
    store(out + 8, _m_punpckhdq(xz_plus, minus));
    store(out + 4, _m_punpckldq(minus, repeat_high(diagonal)));
    _m_femms();
}
} // namespace nfs::graphics::d3dx
