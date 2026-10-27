#include "nfs/audio/backend/sound_backend.hpp"
#include <mm3dnow.h>
#include <cstring>
namespace nfs::sound::backend::pending {
extern Byte interpolation_state[352];
namespace {
__m64 saved(unsigned offset) {
    return field<__m64>(interpolation_state, offset);
}
__m64 two(float a, float b) {
    Dword pair[2];
    std::memcpy(pair, &a, 4);
    std::memcpy(pair + 1, &b, 4);
    return *reinterpret_cast<__m64 *>(pair);
}
void advance(Dword &phase, int &index, int whole, Dword fraction) {
    Dword next = phase + fraction;
    index = int(Dword(index) + Dword(whole) + (next < phase));
    phase = next;
}
__m64 cubic(__m64 a, __m64 b, __m64 c, __m64 d, __m64 t) {
    auto p = _m_pfmul(_m_pfadd(_m_pfsub(_m_pfmul(_m_pfsub(b, c), saved(32)), a), d), saved(16));
    auto q = _m_pfsub(_m_pfadd(_m_pfmul(saved(24), c), a),
                      _m_pfmul(_m_pfadd(_m_pfmul(saved(40), b), d), saved(16)));
    auto r = _m_pfmul(_m_pfsub(c, a), saved(16));
    r = _m_pfmul(r, t);
    t = _m_pfmul(t, t);
    q = _m_pfmul(q, t);
    t = _m_pfmul(t, t);
    p = _m_pfmul(p, t);
    return _m_pfadd(_m_pfadd(b, p), _m_pfadd(r, q));
}
__m64 ratio(Dword phase, unsigned offset) {
    field<Dword>(interpolation_state, offset) = phase;
    auto bits = field<Dword>(interpolation_state, offset + 2);
    return _m_pfmul(_m_pi2fd(_m_from_int(int(bits))), saved(offset == 8 ? 0 : 240));
}
} // namespace
// Saved packed polynomial uses the fourth power term.
void cubic_packed(Dword count, const void *input, void *output, int *index, Dword *phase,
                           int whole_step, Dword fraction_step) {
    const auto *s = static_cast<const float *>(input);
    auto *out = static_cast<float *>(output);
    Dword cursor = *phase;
    int i = *index;
    field<Dword>(interpolation_state, 8) = cursor;
    while (count >= 2) {
        auto t0 = ratio(cursor, 8);
        int first = i;
        advance(cursor, i, whole_step, fraction_step);
        auto t1 = ratio(cursor, 8);
        int second = i;
        auto a = two(s[first - 1], s[second - 1]), b = two(s[first], s[second]),
             c = two(s[first + 1], s[second + 1]), d = two(s[first + 2], s[second + 2]);
        auto t = _m_punpckldq(t0, t1);
        advance(cursor, i, whole_step, fraction_step);
        field<Dword>(interpolation_state, 8) = cursor;
        *reinterpret_cast<__m64 *>(out) = cubic(a, b, c, d, t);
        out += 2;
        count -= 2;
    }
    if (count) {
        auto t = ratio(cursor, 8);
        auto r = cubic(two(s[i - 1], 0), two(s[i], 0), two(s[i + 1], 0), two(s[i + 2], 0), t);
        advance(cursor, i, whole_step, fraction_step);
        field<Dword>(out, 0) = Dword(_m_to_int(r));
    }
    field<Dword>(interpolation_state, 8) = cursor;
    _m_femms();
    *phase = cursor;
    *index = i;
}

void linear_packed(Dword count, const void *input, void *output, int *index, Dword *phase,
                            int whole_step, Dword fraction_step) {
    const auto *s = static_cast<const float *>(input);
    auto *out = static_cast<float *>(output);
    Dword cursor = *phase;
    int i = *index;
    field<Dword>(interpolation_state, 248) = cursor;
    while (count >= 2) {
        auto t0 = ratio(cursor, 248);
        auto a0 = two(s[i], 0), b0 = two(s[i + 1], 0);
        advance(cursor, i, whole_step, fraction_step);
        auto t1 = ratio(cursor, 248);
        auto a1 = two(s[i], 0), b1 = two(s[i + 1], 0);
        auto r0 = _m_pfadd(_m_pfmul(t0, _m_pfsub(b0, a0)), a0),
             r1 = _m_pfadd(_m_pfmul(t1, _m_pfsub(b1, a1)), a1);
        field<Dword>(out, 0) = Dword(_m_to_int(r0));
        field<Dword>(out, 4) = Dword(_m_to_int(r1));
        out += 2;
        advance(cursor, i, whole_step, fraction_step);
        count -= 2;
    }
    if (count) {
        auto t = ratio(cursor, 248);
        auto a = two(s[i], 0), b = two(s[i + 1], 0);
        auto r = _m_pfadd(_m_pfmul(t, _m_pfsub(b, a)), a);
        advance(cursor, i, whole_step, fraction_step);
        field<Dword>(out, 0) = Dword(_m_to_int(r));
    }
    field<Dword>(interpolation_state, 248) = cursor;
    _m_femms();
    *phase = cursor;
    *index = i;
}
} // namespace nfs::sound::backend::pending
