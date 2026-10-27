#include "d3dx_dispatch_kernels.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;
namespace {
void normalize_four(float *out, const float *input) {
    auto xy = load(input), zw = load(input + 2);
    auto square = _m_pfadd(_m_pfmul(xy, xy), _m_pfmul(zw, zw));
    square = _m_pfacc(square, square);
    auto estimate = _m_pfrsqrt(square);
    auto mask = _m_pfcmpgt(square, bits(8388608, 8388608));
    auto inverse = _m_pfrcpit2(_m_pfrsqit1(square, _m_pfmul(estimate, estimate)), estimate);
    store(out, _m_pfmul(_m_pand(xy, mask), inverse));
    store(out + 2, _m_pfmul(_m_pand(zw, mask), inverse));
}
} // namespace

void __stdcall normalize_vector3_packed(float *out, const float *input) {
    _m_femms();
    auto xy = load(input), z = low(input + 2);
    auto square = _m_pfadd(_m_pfmul(xy, xy), _m_pfmul(z, z));
    square = _m_pfacc(square, square);
    auto estimate = _m_pfrsqrt(square);
    auto mask = _m_pfcmpgt(square, bits(8388608, 8388608));
    auto inverse = _m_pfrcpit2(_m_pfrsqit1(square, _m_pfmul(estimate, estimate)), estimate);
    store(out, _m_pfmul(_m_pand(xy, mask), inverse));
    store_low(out + 2, _m_pfmul(_m_pand(z, mask), inverse));
    _m_femms();
}

void __stdcall normalize_vector4_packed(float *out, const float *input) {
    _m_femms();
    normalize_four(out, input);
    _m_femms();
}

void __stdcall normalize_quaternion_packed(float *out, const float *input) {
    normalize_four(out, input);
    _m_femms();
}

void __stdcall normalize_vector2_packed(float *out, const float *input) {
    auto xy = load(input);
    auto square = _m_pfmul(xy, xy);
    square = _m_pfacc(square, square);
    auto estimate = _m_pfrsqrt(square);
    auto mask = _m_pfcmpgt(square, bits(8388608, 8388608));
    // The saved two-component body reverses these two refinement operands.
    auto inverse = _m_pfrcpit2(_m_pfrsqit1(_m_pfmul(estimate, estimate), square), estimate);
    store(out, _m_pfmul(_m_pand(xy, mask), inverse));
    _m_femms();
}

void __stdcall normalize_plane_packed(float *out, const float *input) {
    auto xy = load(input), zw = load(input + 2), z = _m_pand(zw, bits(0, -1));
    auto square = _m_pfadd(_m_pfmul(xy, xy), _m_pfmul(z, z));
    square = _m_pfacc(square, square);
    auto estimate = _m_pfrsqrt(square);
    auto mask = _m_pfcmpgt(square, bits(8388608, 8388608));
    auto inverse = _m_pfrcpit2(_m_pfrsqit1(square, _m_pfmul(estimate, estimate)), estimate);
    store(out, _m_pfmul(_m_pand(xy, mask), inverse));
    store(out + 2, _m_pfmul(_m_pand(zw, mask), inverse));
    _m_femms();
}

void __stdcall plane_from_point_normal_packed(float *out, const float *point, const float *normal) {
    _m_femms();
    auto xy = _m_pfmul(load(point), load(normal));
    auto z = _m_pfmul(low(point + 2), low(normal + 2));
    auto dot = _m_pfadd(_m_pfacc(xy, xy), z);
    store(out, load(normal));
    store_low(out + 2, low(normal + 2));
    store_low(out + 3, _m_pxor(dot, bits(0, int(2147483648u))));
    _m_femms();
}
} // namespace nfs::graphics::d3dx
