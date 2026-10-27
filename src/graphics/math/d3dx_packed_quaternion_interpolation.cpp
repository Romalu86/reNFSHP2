#include "d3dx_packed_matrix_values.hpp"
#include "d3dx_packed_values.hpp"
#include "d3dx_trigonometric_constants.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;
using trigonometry::approximate_acos_packed;
using trigonometry::approximate_atan2_packed;
using trigonometry::approximate_sine_cosine_packed;
using trigonometry::approximate_sine_packed;

void __stdcall quaternion_axis_angle_packed(const float *rotation, float *axis, float *angle) {
    auto xy = load(rotation), zw = load(rotation + 2);
    auto square = _m_pfadd(_m_pfmul(xy, xy), _m_pfmul(zw, zw));
    square = _m_pfacc(square, square);
    auto tiny = _m_pfcmpge(bits(8388608, 8388608), square);
    auto estimate = _m_pfrsqrt(square);
    if (!_m_to_int(tiny)) {
        auto estimate_square = _m_pfmul(estimate, estimate);
        if (axis) {
            auto inverse = _m_pfrcpit2(_m_pfrsqit1(estimate_square, square), estimate);
            store(axis, _m_pfmul(xy, inverse));
            store_low(axis + 2, _m_pfmul(inverse, zw));
        }
        if (angle)
            store_low(angle, _m_pfmul(approximate_acos_packed(repeat_high(zw)),
                                      bits(1073741824, 1073741824)));
    } else {
        if (axis) {
            store(axis + 1, _m_from_int(0));
            store_low(axis, bits(int(3212836864u), 1065353216));
        }
        if (angle)
            store_low(angle, _m_from_int(0));
    }
    _m_femms();
}

void __stdcall interpolate_quaternion_packed(float *out, const float *first, const float *second,
                                             float fraction) {
    _m_femms();
    auto dot =
        _m_pfadd(_m_pfmul(load(first), load(second)), _m_pfmul(load(first + 2), load(second + 2)));
    dot = _m_pfacc(dot, dot);
    auto second_weight = low(&fraction);
    auto first_weight = _m_pfsub(_m_from_int(1065353216), second_weight);
    auto sign = _m_pand(_m_pfcmpge(_m_from_int(0), dot), bits(int(2147483648u), int(2147483648u)));
    dot = _m_pxor(dot, sign);
    auto spherical = _m_pfcmpge(bits(1065353048, 1065353048), dot);
    auto square = _m_pfsubr(_m_pfmul(dot, dot), bits(1065353216, 1065353216));
    if (_m_to_int(spherical)) {
        auto estimate = _m_pfrsqrt(square);
        auto inverse_sine =
            _m_pfrcpit2(_m_pfrsqit1(square, _m_pfmul(estimate, estimate)), estimate);
        auto sine = reciprocal(inverse_sine);
        float weight, inverse, angle;
        Dword sign_word = Dword(_m_to_int(sign));
        store_low(&weight, first_weight);
        store_low(&inverse, inverse_sine);
        store_low(&angle, approximate_atan2_packed(dot, sine));
        float sine_second;
        store_low(&sine_second, approximate_sine_packed(_m_pfmul(low(&angle), low(&fraction))));
        first_weight = approximate_sine_packed(_m_pfmul(low(&angle), low(&weight)));
        second_weight = low(&sine_second);
        sign = _m_from_int(int(sign_word));
        first_weight = _m_pfmul(first_weight, low(&inverse));
        second_weight = _m_pfmul(second_weight, low(&inverse));
    }
    second_weight = repeat_low(_m_pxor(second_weight, sign));
    first_weight = repeat_low(first_weight);
    auto xy = _m_pfadd(_m_pfmul(first_weight, load(first)), _m_pfmul(second_weight, load(second)));
    auto zw = _m_pfadd(_m_pfmul(first_weight, load(first + 2)),
                       _m_pfmul(second_weight, load(second + 2)));
    store(out, xy);
    store(out + 2, zw);
    _m_femms();
}

float *__stdcall barycentric_quaternion_packed(float *out, const float *first, const float *second,
                                               const float *third, float second_fraction,
                                               float third_fraction) {
    _m_femms();
    float sum;
    store_low(&sum, _m_pfadd(low(&second_fraction), low(&third_fraction)));
    float first_arc[4], second_arc[4];
    interpolate_quaternion_packed(first_arc, first, second, sum);
    interpolate_quaternion_packed(second_arc, first, third, sum);
    float fraction;
    store_low(&fraction, _m_pfmul(reciprocal(low(&sum)), low(&third_fraction)));
    interpolate_quaternion_packed(out, first_arc, second_arc, fraction);
    _m_femms();
    return out;
}

float *__stdcall squad_quaternion_packed(float *out, const float *first, const float *second,
                                         const float *third, const float *fourth, float fraction) {
    float outer[4], inner[4];
    interpolate_quaternion_packed(outer, first, fourth, fraction);
    interpolate_quaternion_packed(inner, second, third, fraction);
    auto t = low(&fraction);
    float blend;
    store_low(&blend, _m_pfmul(_m_pfsub(t, _m_pfmul(t, t)), bits(1073741824, 1073741824)));
    interpolate_quaternion_packed(out, outer, inner, blend);
    _m_femms();
    return out;
}
void packed::logarithm_quaternion_values(float *out, const float *input) {
    auto xy = load(input), zw = load(input + 2);
    if (!_m_to_int(_m_pfcmpge(repeat_high(zw), bits(1065353216, 1065353216)))) {
        float angle;
        auto value = approximate_acos_packed(repeat_high(zw));
        store_low(&angle, value);
        auto sine = approximate_sine_packed(value);
        auto enough =
            _m_pfcmpge(_m_pand(sine, bits(2147483647, 2147483647)), bits(872415232, 872415232));
        xy = load(input);
        zw = load(input + 2);
        if (_m_to_int(enough)) {
            auto scale = _m_pfmul(repeat_low(low(&angle)), reciprocal(sine));
            xy = _m_pfmul(xy, scale);
            zw = _m_pfmul(zw, scale);
        }
    }
    store(out, xy);
    store(out + 2, _m_pand(zw, bits(0, -1)));
}

void __stdcall logarithm_quaternion_packed(float *out, const float *input) {
    _m_femms();
    logarithm_quaternion_values(out, input);
    _m_femms();
}
void packed::exponential_quaternion_values(float *out, const float *input) {
    auto xy = load(input), z = low(input + 2);
    auto square = _m_pfadd(_m_pfmul(xy, xy), _m_pfmul(z, z));
    square = _m_pfacc(square, square);
    auto estimate = _m_pfrsqrt(square);
    auto inverse = _m_pfrcpit2(_m_pfrsqit1(_m_pfmul(estimate, estimate), square), estimate);
    float inverse_length;
    store_low(&inverse_length, inverse);
    auto pair = approximate_sine_cosine_packed(_m_pfmul(square, inverse));
    auto enough = _m_pfcmpge(_m_pand(pair, bits(2147483647, -1)), bits(872415232, 872415232));
    xy = load(input);
    z = low(input + 2);
    auto sine = repeat_high(pair);
    auto scale = _m_pfmul(sine, low(&inverse_length));
    if (Dword(_m_to_int(repeat_high(enough))) & 1) {
        scale = repeat_low(scale);
        z = _m_pfmul(z, scale);
        xy = _m_pfmul(xy, scale);
    }
    store(out, xy);
    store(out + 2, _m_punpckldq(z, pair));
}

float *__stdcall exponential_quaternion_packed(float *out, const float *input) {
    exponential_quaternion_values(out, input);
    _m_femms();
    return out;
}
} // namespace nfs::graphics::d3dx
