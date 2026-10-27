#include "d3dx_dispatch_kernels.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;
namespace {
template <unsigned Size>
void barycentric(float *out, const float *origin, const float *first, const float *second, float u,
                 float v) {
    auto a = load(origin), wu = splat(&u), wv = splat(&v);
    auto first_delta = _m_pfsub(load(first), a), second_delta = _m_pfsub(load(second), a);
    __m64 tail;
    if constexpr (Size > 2) {
        auto z = Size == 3 ? low(origin + 2) : load(origin + 2);
        auto b = Size == 3 ? low(first + 2) : load(first + 2);
        auto c = Size == 3 ? low(second + 2) : load(second + 2);
        tail = _m_pfadd(_m_pfadd(_m_pfmul(_m_pfsub(b, z), wu), z), _m_pfmul(_m_pfsub(c, z), wv));
    }
    auto result = _m_pfadd(_m_pfadd(_m_pfmul(first_delta, wu), a), _m_pfmul(second_delta, wv));
    store(out, result);
    if constexpr (Size == 3)
        store_low(out + 2, tail);
    if constexpr (Size == 4)
        store(out + 2, tail);
}
template <unsigned Size>
void hermite(float *out, const float *first, const float *first_tangent, const float *second,
             const float *second_tangent, float time) {
    auto t = low(&time), square = _m_pfmul(t, t), cube = _m_pfmul(square, t);
    auto two = bits(1073741824, 1073741824), three = bits(1077936128, 1077936128);
    auto second_tangent_weight = repeat_low(_m_pfsub(cube, square));
    auto first_tangent_weight = repeat_low(_m_pfadd(_m_pfsub(cube, _m_pfmul(square, two)), t));
    auto triple_square = _m_pfmul(square, three), twice_cube = _m_pfmul(cube, two);
    auto second_weight = repeat_low(_m_pfsub(triple_square, twice_cube));
    auto first_weight =
        repeat_low(_m_pfadd(_m_pfsub(twice_cube, triple_square), bits(1065353216, 1065353216)));
    auto evaluate = [&](unsigned offset, bool single) {
        auto read = [&](const float *p) { return single ? low(p + offset) : load(p + offset); };
        auto first_half = _m_pfadd(_m_pfmul(first_weight, read(first)),
                                   _m_pfmul(first_tangent_weight, read(first_tangent)));
        auto second_half = _m_pfadd(_m_pfmul(second_weight, read(second)),
                                    _m_pfmul(second_tangent_weight, read(second_tangent)));
        if constexpr (Size == 2)
            return _m_pfadd(first_half, second_half);
        else
            return _m_pfadd(second_half, first_half);
    };
    auto result = evaluate(0, false);
    __m64 tail;
    if constexpr (Size > 2)
        tail = evaluate(2, Size == 3);
    store(out, result);
    if constexpr (Size == 3)
        store_low(out + 2, tail);
    if constexpr (Size == 4)
        store(out + 2, tail);
}
template <unsigned Size>
void catmull(float *out, const float *previous, const float *first, const float *second,
             const float *next, float time) {
    auto t = splat(&time), time_one = _m_punpckldq(low(&time), bits(0, 1065353216));
    auto square = _m_pfmul(time_one, time_one);
    auto difference = _m_pfsub(_m_pfmul(_m_pfmul(t, t), time_one), square);
    auto scaled = _m_pfmul(difference, bits(1073741824, 1077936128));
    auto mixed = _m_punpckldq(scaled, difference);
    auto signed_time = _m_pxor(t, bits(int(2147483648u), 0));
    auto middle = _m_pfsub(_m_pfadd(repeat_low(square), signed_time), mixed);
    auto outer = _m_pfsub(mixed, _m_psrlqi(scaled, 32));
    auto previous_weight = repeat_high(middle), first_weight = repeat_low(outer);
    auto second_weight = repeat_low(middle), next_weight = repeat_high(outer);
    auto half = bits(1056964608, 1056964608);
    auto evaluate = [&](unsigned offset, bool single) {
        auto read = [&](const float *p) { return single ? low(p + offset) : load(p + offset); };
        auto a = _m_pfmul(previous_weight, read(previous));
        auto b = _m_pfmul(first_weight, read(first));
        auto sum = offset ? _m_pfadd(a, b) : _m_pfadd(b, a);
        sum = _m_pfadd(sum, _m_pfmul(second_weight, read(second)));
        sum = _m_pfadd(sum, _m_pfmul(next_weight, read(next)));
        return _m_pfmul(sum, half);
    };
    auto result = evaluate(0, false);
    __m64 tail;
    if constexpr (Size > 2)
        tail = evaluate(2, Size == 3);
    store(out, result);
    if constexpr (Size == 3)
        store_low(out + 2, tail);
    if constexpr (Size == 4)
        store(out + 2, tail);
}
} // namespace

void __stdcall barycentric_vector2_packed(float *out, const float *origin, const float *first,
                                          const float *second, float u, float v) {
    barycentric<2>(out, origin, first, second, u, v);
    _m_femms();
}

void __stdcall barycentric_vector3_packed(float *out, const float *origin, const float *first,
                                          const float *second, float u, float v) {
    barycentric<3>(out, origin, first, second, u, v);
    _m_femms();
}

void __stdcall barycentric_vector4_packed(float *out, const float *origin, const float *first,
                                          const float *second, float u, float v) {
    barycentric<4>(out, origin, first, second, u, v);
    _m_femms();
}

void __stdcall hermite_vector2_packed(float *out, const float *first, const float *first_tangent,
                                      const float *second, const float *second_tangent,
                                      float time) {
    _m_femms();
    hermite<2>(out, first, first_tangent, second, second_tangent, time);
    _m_femms();
}

void __stdcall hermite_vector3_packed(float *out, const float *first, const float *first_tangent,
                                      const float *second, const float *second_tangent,
                                      float time) {
    _m_femms();
    hermite<3>(out, first, first_tangent, second, second_tangent, time);
    _m_femms();
}

void __stdcall hermite_vector4_packed(float *out, const float *first, const float *first_tangent,
                                      const float *second, const float *second_tangent,
                                      float time) {
    _m_femms();
    hermite<4>(out, first, first_tangent, second, second_tangent, time);
    _m_femms();
}

void __stdcall catmull_vector2_packed(float *out, const float *previous, const float *first,
                                      const float *second, const float *next, float time) {
    catmull<2>(out, previous, first, second, next, time);
    _m_femms();
}

void __stdcall catmull_vector3_packed(float *out, const float *previous, const float *first,
                                      const float *second, const float *next, float time) {
    catmull<3>(out, previous, first, second, next, time);
    _m_femms();
}

void __stdcall catmull_vector4_packed(float *out, const float *previous, const float *first,
                                      const float *second, const float *next, float time) {
    catmull<4>(out, previous, first, second, next, time);
    _m_femms();
}
} // namespace nfs::graphics::d3dx
