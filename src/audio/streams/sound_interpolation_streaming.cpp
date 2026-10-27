#include "nfs/audio/backend/sound_backend.hpp"
#include <xmmintrin.h>
namespace nfs::sound::backend::pending {
extern Byte interpolation_state[352];
namespace {
float *values(unsigned offset) {
    return reinterpret_cast<float *>(interpolation_state + offset);
}
__m128 saved(unsigned offset) {
    return _mm_load_ps(values(offset));
}
__m128 convert(unsigned offset) {
    auto *p = interpolation_state + offset;
    auto hi = _mm_cvtpi32_ps(_mm_setzero_ps(), field<__m64>(p, 8));
    hi = _mm_shuffle_ps(hi, hi, 64);
    auto lo = _mm_cvtpi32_ps(hi, field<__m64>(p, 0));
    return _mm_shuffle_ps(lo, lo, 228);
}
void advance(Dword &phase, int &index, int whole, Dword fraction) {
    phase &= 4294967294u;
    Dword next = phase + fraction;
    index = int(Dword(index) + Dword(whole) + (next < phase));
    phase = next;
}
__m128 cubic(__m128 a, __m128 b, __m128 c, __m128 d, __m128 t) {
    auto p = _mm_mul_ps(_mm_add_ps(_mm_sub_ps(_mm_mul_ps(_mm_sub_ps(b, c), saved(176)), a), d),
                        saved(144));
    auto q = _mm_sub_ps(_mm_add_ps(_mm_mul_ps(saved(160), c), a),
                        _mm_mul_ps(_mm_add_ps(_mm_mul_ps(saved(192), b), d), saved(144)));
    auto r = _mm_mul_ps(_mm_mul_ps(_mm_sub_ps(c, a), saved(144)), t);
    t = _mm_mul_ps(t, t);
    q = _mm_mul_ps(q, t);
    t = _mm_mul_ps(t, t);
    p = _mm_mul_ps(p, t);
    return _mm_add_ps(_mm_add_ps(b, p), _mm_add_ps(r, q));
}
} // namespace
// The saved route truncates phase bit zero before advancing.
void cubic_streaming(Dword count, const void *input, void *output, int *index, Dword *phase,
                              int whole_step, Dword fraction_step) {
    const auto *s = static_cast<const float *>(input);
    auto *out = static_cast<float *>(output);
    Dword cursor = *phase;
    int i = *index;
    Dword frame_marker;
    field<Dword>(interpolation_state, 208) = reinterpret_cast<Dword>(&frame_marker);
    while (count >= 4) {
        for (unsigned lane = 0; lane < 4; ++lane) {
            field<Dword>(interpolation_state, 128 + lane * 4) = cursor >> 1;
            values(64)[lane] = s[i - 1];
            values(80)[lane] = s[i];
            values(96)[lane] = s[i + 1];
            values(112)[lane] = s[i + 2];
            advance(cursor, i, whole_step, fraction_step);
        }
        auto t = _mm_mul_ps(convert(128), saved(48));
        auto r = cubic(saved(64), saved(80), saved(96), saved(112), t);
        _mm_store_ps(out, r);
        out += 4;
        count -= 4;
    }
    while (count--) {
        field<Dword>(interpolation_state, 128) = cursor >> 1;
        auto a = _mm_set_ss(s[i - 1]), b = _mm_set_ss(s[i]), c = _mm_set_ss(s[i + 1]),
             d = _mm_set_ss(s[i + 2]);
        advance(cursor, i, whole_step, fraction_step);
        auto t = _mm_mul_ps(convert(128), saved(48));
        _mm_store_ss(out++, cubic(a, b, c, d, t));
    }
    *phase = cursor;
    *index = i;
}
// Preserve eight-sample collection before either output store.
void linear_streaming(Dword count, const void *input, void *output, int *index, Dword *phase,
                               int whole_step, Dword fraction_step) {
    const auto *s = static_cast<const float *>(input);
    auto *out = static_cast<float *>(output);
    Dword cursor = *phase;
    int i = *index;
    auto scale = saved(256);
    while (count >= 8) {
        __m128 first_next;
        for (unsigned lane = 0; lane < 8; ++lane) {
            field<Dword>(interpolation_state, 320 + lane * 4) = cursor >> 1;
            values(304)[lane & 3] = s[i + 1];
            values(lane < 4 ? 272 : 288)[lane & 3] = s[i];
            advance(cursor, i, whole_step, fraction_step);
            if (lane == 3)
                first_next = saved(304);
        }
        auto t0 = _mm_mul_ps(convert(320), scale), t1 = _mm_mul_ps(convert(336), scale);
        auto a0 = saved(272), a1 = saved(288), b1 = saved(304);
        auto r0 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(first_next, a0), t0), a0),
             r1 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(b1, a1), t1), a1);
        _mm_store_ps(out, r0);
        _mm_store_ps(out + 4, r1);
        out += 8;
        count -= 8;
    }
    while (count--) {
        auto t = _mm_mul_ss(_mm_cvtsi32_ss(_mm_setzero_ps(), int(cursor >> 1)), scale),
             a = _mm_set_ss(s[i]), b = _mm_set_ss(s[i + 1]);
        advance(cursor, i, whole_step, fraction_step);
        auto r = _mm_add_ss(a, _mm_mul_ss(_mm_sub_ss(b, a), t));
        _mm_store_ss(out++, r);
    }
    *phase = cursor;
    *index = i;
}
} // namespace nfs::sound::backend::pending
