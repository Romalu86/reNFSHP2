#include "d3dx_dispatch_kernels.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;

void __stdcall transform_point2_packed(float *out, const float *point, const float *matrix) {
    _m_femms();
    auto source = load(point), x = repeat_low(source), y = repeat_high(source);
    auto xy = _m_pfadd(_m_pfadd(_m_pfmul(x, load(matrix)), load(matrix + 12)),
                       _m_pfmul(y, load(matrix + 4)));
    auto zw = _m_pfadd(_m_pfadd(_m_pfmul(x, load(matrix + 2)), load(matrix + 14)),
                       _m_pfmul(y, load(matrix + 6)));
    store(out, xy);
    store(out + 2, zw);
    _m_femms();
}

void __stdcall transform_point3_packed(float *out, const float *point, const float *matrix) {
    _m_femms();
    auto source = load(point), x = repeat_low(source), y = repeat_high(source),
         z = splat(point + 2);
    auto xy = _m_pfadd(_m_pfadd(_m_pfmul(x, load(matrix)), load(matrix + 12)),
                       _m_pfadd(_m_pfmul(y, load(matrix + 4)), _m_pfmul(z, load(matrix + 8))));
    auto zw = _m_pfadd(_m_pfadd(_m_pfmul(x, load(matrix + 2)), load(matrix + 14)),
                       _m_pfadd(_m_pfmul(y, load(matrix + 6)), _m_pfmul(z, load(matrix + 10))));
    store(out, xy);
    store(out + 2, zw);
    _m_femms();
}

void __stdcall transform_vector4_packed(float *out, const float *point, const float *matrix) {
    _m_femms();
    auto first = load(point), second = load(point + 2);
    auto x = repeat_low(first), y = repeat_high(first);
    auto z = repeat_low(second), w = repeat_high(second);
    auto xy = _m_pfadd(_m_pfadd(_m_pfmul(x, load(matrix)), _m_pfmul(y, load(matrix + 4))),
                       _m_pfadd(_m_pfmul(z, load(matrix + 8)), _m_pfmul(w, load(matrix + 12))));
    auto zw = _m_pfadd(_m_pfadd(_m_pfmul(x, load(matrix + 2)), _m_pfmul(y, load(matrix + 6))),
                       _m_pfadd(_m_pfmul(z, load(matrix + 10)), _m_pfmul(w, load(matrix + 14))));
    store(out, xy);
    store(out + 2, zw);
    _m_femms();
}

void __stdcall transform_normal2_packed(float *out, const float *point, const float *matrix) {
    _m_femms();
    auto source = load(point);
    store(out, _m_pfadd(_m_pfmul(repeat_low(source), load(matrix)),
                        _m_pfmul(repeat_high(source), load(matrix + 4))));
    _m_femms();
}

void __stdcall transform_normal3_packed(float *out, const float *point, const float *matrix) {
    _m_femms();
    auto source = load(point), x = repeat_low(source), y = repeat_high(source),
         z = splat(point + 2);
    auto xy = _m_pfadd(_m_pfmul(x, load(matrix)),
                       _m_pfadd(_m_pfmul(y, load(matrix + 4)), _m_pfmul(z, load(matrix + 8))));
    auto zw = _m_pfadd(_m_pfmul(x, load(matrix + 2)),
                       _m_pfadd(_m_pfmul(y, load(matrix + 6)), _m_pfmul(z, load(matrix + 10))));
    store(out, xy);
    store_low(out + 2, zw);
    _m_femms();
}

void __stdcall transform_coordinate2_packed(float *out, const float *point, const float *matrix) {
    _m_femms();
    auto source = load(point), x = repeat_low(source), y = repeat_high(source);
    auto xy = _m_pfadd(_m_pfadd(_m_pfmul(x, load(matrix)), load(matrix + 12)),
                       _m_pfmul(y, load(matrix + 4)));
    auto zw = _m_pfadd(_m_pfadd(_m_pfmul(x, load(matrix + 2)), load(matrix + 14)),
                       _m_pfmul(y, load(matrix + 6)));
    store(out, _m_pfmul(xy, reciprocal(repeat_high(zw))));
    _m_femms();
}

void __stdcall transform_projected_point3_packed(float *out, const float *point,
                                                 const float *matrix) {
    _m_femms();
    auto source = load(point), x = repeat_low(source), y = repeat_high(source),
         z = splat(point + 2);
    auto xy = _m_pfadd(_m_pfadd(_m_pfmul(x, load(matrix)), load(matrix + 12)),
                       _m_pfadd(_m_pfmul(y, load(matrix + 4)), _m_pfmul(z, load(matrix + 8))));
    auto zw = _m_pfadd(_m_pfadd(_m_pfmul(x, load(matrix + 2)), load(matrix + 14)),
                       _m_pfadd(_m_pfmul(y, load(matrix + 6)), _m_pfmul(z, load(matrix + 10))));
    auto inverse = reciprocal(repeat_high(zw));
    store(out, _m_pfmul(xy, inverse));
    store_low(out + 2, _m_pfmul(zw, inverse));
    _m_femms();
}

void __stdcall transform_coordinate3_packed(float *out, const float *point, const float *matrix) {
    transform_projected_point3_packed(out, point, matrix);
}
// Packed arithmetic and memory operations match .
void __stdcall transform_plane_packed(float *out, const float *plane, const float *matrix) {
    transform_vector4_packed(out, plane, matrix);
}
} // namespace nfs::graphics::d3dx
