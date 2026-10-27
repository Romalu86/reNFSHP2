#include "nfs/audio/backend/sound_ownership.hpp"
#include <mm3dnow.h>
#include <xmmintrin.h>
#include <cstring>
namespace nfs::sound::backend::pending {

void mix_streaming(int count, float gain, const float *input, float *output) {
    Dword n = Dword(count);
    auto scalar = _mm_set_ss(gain), factor = _mm_set1_ps(gain);
    while ((n & 15) || (reinterpret_cast<Dword>(input) & 15) ||
           (reinterpret_cast<Dword>(output) & 15)) {
        auto r =
            _mm_add_ss(_mm_mul_ss(_mm_load_ss(input + n - 1), scalar), _mm_load_ss(output + n - 1));
        _mm_store_ss(output + n - 1, r);
        if (!--n)
            return;
    }
    do {
        Dword p = n - 16;
        __m128 x[4], y[4];
        for (unsigned j = 0; j < 4; ++j) {
            x[j] = _mm_load_ps(input + p + j * 4);
            y[j] = _mm_load_ps(output + p + j * 4);
        }
        for (unsigned j = 0; j < 4; ++j)
            _mm_store_ps(output + p + j * 4, _mm_add_ps(_mm_mul_ps(x[j], factor), y[j]));
        Dword before = n;
        n -= 16;
        if (!n || int(before) <= 15)
            break;
    } while (true);
}

void mix_packed(int count, float gain, const float *input, float *output) {
    Dword bits;
    std::memcpy(&bits, &gain, 4);
    __m64 factor = _m_punpckldq(_m_from_int(int(bits)), _m_from_int(int(bits)));
    Dword n = Dword(count);
    while (n & 7) {
        Dword in, out;
        std::memcpy(&in, input + n - 1, 4);
        std::memcpy(&out, output + n - 1, 4);
        auto r = _m_pfadd(_m_pfmul(_m_from_int(int(in)), factor), _m_from_int(int(out)));
        out = Dword(_m_to_int(r));
        std::memcpy(output + n - 1, &out, 4);
        if (!--n) {
            _m_femms();
            return;
        }
    }
    do {
        Dword p = n - 8;
        __m64 products[4];
        for (unsigned j = 0; j < 4; ++j)
            products[j] = _m_pfmul(*reinterpret_cast<const __m64 *>(input + p + j * 2), factor);
        for (unsigned j = 0; j < 4; ++j)
            *reinterpret_cast<__m64 *>(output + p + j * 2) =
                _m_pfadd(products[j], *reinterpret_cast<const __m64 *>(output + p + j * 2));
        Dword before = n;
        n -= 8;
        if (!n || int(before) <= 7)
            break;
    } while (true);
    _m_femms();
}

void pcm16_packed(int count, const std::int16_t *input, float *output) {
    Dword n = Dword(count);
    while (n & 7) {
        auto r = _m_pi2fd(_m_from_int(int(input[n - 1])));
        Dword bits = Dword(_m_to_int(r));
        std::memcpy(output + n - 1, &bits, 4);
        if (!--n) {
            _m_femms();
            return;
        }
    }
    do {
        Dword p = n - 8;
        std::int16_t v[8];
        std::memcpy(v, input + p, 16);
        for (unsigned j = 0; j < 4; ++j) {
            __m64 pair = _m_punpckldq(_m_from_int(v[j * 2]), _m_from_int(v[j * 2 + 1]));
            *reinterpret_cast<__m64 *>(output + p + j * 2) = _m_pi2fd(pair);
        }
        Dword before = n;
        n -= 8;
        if (!n || int(before) <= 7)
            break;
    } while (true);
    _m_femms();
}
} // namespace nfs::sound::backend::pending
