#include "d3dx_packed_matrix_values.hpp"
#include "d3dx_packed_values.hpp"
#include "d3dx_trigonometric_constants.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;
using trigonometry::approximate_sine_cosine_packed;

void __stdcall rotation_x_matrix_packed(float *out, float angle) {
    _m_femms();
    auto pair = approximate_sine_cosine_packed(low(&angle));
    auto sine = _m_psrlqi(pair, 32), zero = _m_from_int(0);
    store(out, bits(0, 1065353216));
    store(out + 2, zero);
    store_low(out + 4, zero);
    store_low(out + 5, pair);
    store(out + 6, sine);
    sine = _m_pxor(sine, bits(0, int(2147483648u)));
    store(out + 12, zero);
    store(out + 14, bits(1065353216, 0));
    store_low(out + 8, zero);
    store_low(out + 9, sine);
    store_low(out + 10, pair);
    store_low(out + 11, zero);
    _m_femms();
}

void __stdcall rotation_y_matrix_packed(float *out, float angle) {
    _m_femms();
    auto pair = approximate_sine_cosine_packed(low(&angle));
    auto cosine = _m_pand(pair, bits(0, -1)), sine = _m_psrlqi(pair, 32), zero = _m_from_int(0);
    store(out + 4, bits(1065353216, 0));
    store(out + 6, zero);
    store(out + 12, zero);
    store(out + 14, bits(1065353216, 0));
    store(out + 8, sine);
    sine = _m_pxor(sine, bits(0, int(2147483648u)));
    store(out, cosine);
    store(out + 10, cosine);
    store(out + 2, sine);
    _m_femms();
}

void __stdcall rotation_z_matrix_packed(float *out, float angle) {
    _m_femms();
    auto pair = approximate_sine_cosine_packed(low(&angle));
    auto reversed = _m_punpckldq(repeat_high(pair), pair);
    reversed = _m_pxor(reversed, bits(0, int(2147483648u)));
    auto zero = _m_from_int(0);
    store(out, pair);
    store(out + 2, zero);
    store(out + 6, zero);
    store(out + 8, zero);
    store(out + 10, bits(0, 1065353216));
    store(out + 12, zero);
    store(out + 14, bits(1065353216, 0));
    store(out + 4, reversed);
    _m_femms();
}

float *__stdcall quaternion_from_axis_packed(float *out, const float *axis, float angle) {
    _m_femms();
    float normal[3];
    normalize_vector3_packed(normal, axis);
    auto half = _m_pfmul(low(&angle), bits(1056964608, 1056964608));
    auto pair = approximate_sine_cosine_packed(half), sine = repeat_high(pair);
    auto xy = _m_pfmul(load(normal), sine), z = _m_pfmul(low(normal + 2), sine);
    store(out, xy);
    store(out + 2, _m_punpckldq(z, pair));
    _m_femms();
    return out;
}
void packed::yaw_pitch_roll_values(float *out, float yaw, float pitch, float roll) {
    auto half = bits(1056964608, 1056964608);
    auto half_yaw_pitch = _m_pfmul(half, _m_punpckldq(low(&yaw), low(&pitch)));
    auto half_roll = _m_pfmul(half, low(&roll));
    auto roll_pair = approximate_sine_cosine_packed(half_roll);
    auto pitch_pair = approximate_sine_cosine_packed(repeat_high(half_yaw_pitch));
    auto yaw_pair = approximate_sine_cosine_packed(half_yaw_pitch);
    auto cy = repeat_low(yaw_pair), sy = repeat_high(yaw_pair);
    auto cp = repeat_low(pitch_pair), sp = repeat_high(pitch_pair);
    auto swapped_roll = _m_punpckldq(repeat_high(roll_pair), roll_pair);
    auto first = _m_pfmul(_m_pxor(cy, bits(int(2147483648u), 0)), sp);
    auto second = _m_pfmul(cy, cp);
    auto third = _m_pfmul(sy, cp);
    auto fourth = _m_pfmul(_m_pxor(sy, bits(0, int(2147483648u))), sp);
    first = _m_pfmul(first, roll_pair);
    second = _m_pfmul(second, swapped_roll);
    third = _m_pfmul(third, swapped_roll);
    fourth = _m_pfmul(fourth, roll_pair);
    store(out, _m_pfadd(first, third));
    store(out + 2, _m_pfadd(second, fourth));
}

void __stdcall quaternion_yaw_pitch_roll_packed(float *out, float yaw, float pitch, float roll) {
    _m_femms();
    yaw_pitch_roll_values(out, yaw, pitch, roll);
    _m_femms();
}
} // namespace nfs::graphics::d3dx
