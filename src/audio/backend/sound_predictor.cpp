#include "nfs/audio/codecs/sound_codecs.hpp"
#include <xmmintrin.h>
#include <mm3dnow.h>
#include <cstring>
namespace nfs::sound::backend::codecs {
Byte predictor_features{};

void decode_predictor_streaming(void *data) {
    auto *r = static_cast<Dword *>(data);
    auto *input = reinterpret_cast<const Byte *>(r[3]);
    auto *out = reinterpret_cast<float *>(r[4]);
    auto previous = _mm_set_ss(field<float>(r, 4)), older = _mm_set_ss(field<float>(r, 8));
    while (int(r[0]) > 0) {
        r[0] -= 28;
        unsigned filter = *input >> 4, scale = (*input & 15) * 64;
        auto a = _mm_set_ss(table<float>(7130376 + filter * 4)),
             b = _mm_set_ss(table<float>(7130392 + filter * 4));
        for (unsigned j = 0; j < 14; ++j) {
            Byte v = input[j + 1];
            auto first =
                _mm_add_ss(_mm_add_ss(_mm_mul_ss(previous, a),
                                      _mm_set_ss(table<float>(7130408 + scale + (v >> 4) * 4))),
                           _mm_mul_ss(older, b));
            auto second =
                _mm_add_ss(_mm_add_ss(_mm_mul_ss(first, a),
                                      _mm_set_ss(table<float>(7130408 + scale + (v & 15) * 4))),
                           _mm_mul_ss(previous, b));
            _mm_store_ss(out, first);
            _mm_store_ss(out + 1, second);
            older = first;
            previous = second;
            out += 2;
        }
        input += 15;
    }
    _mm_store_ss(reinterpret_cast<float *>(r + 1), previous);
    _mm_store_ss(reinterpret_cast<float *>(r + 2), older);
    r[4] = reinterpret_cast<Dword>(out);
}
// The first packed sum has its own saved association.
void decode_predictor_packed(void *data) {
    auto *r = static_cast<Dword *>(data);
    auto *input = reinterpret_cast<const Byte *>(r[3]);
    auto *out = reinterpret_cast<Dword *>(r[4]);
    auto previous = _m_from_int(int(r[1])), older = _m_from_int(int(r[2]));
    auto load = [](unsigned va) { return _m_from_int(int(table<Dword>(va))); };
    while (int(r[0]) > 0) {
        r[0] -= 28;
        unsigned filter = *input >> 4, scale = (*input & 15) * 64;
        auto a = load(7130376 + filter * 4), b = load(7130392 + filter * 4);
        for (unsigned j = 0; j < 14; ++j) {
            Byte v = input[j + 1];
            auto first = _m_pfadd(_m_pfadd(_m_pfmul(previous, a), _m_pfmul(older, b)),
                                  load(7130408 + scale + (v >> 4) * 4));
            auto second =
                _m_pfadd(_m_pfadd(_m_pfmul(first, a), load(7130408 + scale + (v & 15) * 4)),
                         _m_pfmul(previous, b));
            out[0] = Dword(_m_to_int(first));
            out[1] = Dword(_m_to_int(second));
            older = first;
            previous = second;
            out += 2;
        }
        input += 15;
    }
    r[1] = Dword(_m_to_int(previous));
    r[2] = Dword(_m_to_int(older));
    r[4] = reinterpret_cast<Dword>(out);
    _m_femms();
}

void *__fastcall create_predictor(void *state, void *) {
    field<Dword>(state, 12) = field<Dword>(state, 8) = field<Dword>(state, 4) =
        field<Dword>(state, 152) = field<Dword>(state, 156) = 0;
    field<void(__cdecl *)(void *)>(state, 0) = predictor_features & 2   ? decode_predictor_streaming
                                               : predictor_features & 4 ? decode_predictor_packed
                                                                        : decode_predictor_scalar;
    return state;
}

int __fastcall bind_predictor(void *state, void *, const void *input, Dword bytes, int samples) {
    if (!input || field<Dword>(state, 8))
        return -1;
    field<const void *>(state, 160) = input;
    field<int>(state, 8) = samples;
    field<Dword>(state, 12) = bytes;
    return 0;
}

int __fastcall read_predictor(void *state, void *, float **output, int requested) {
    auto *r = static_cast<Dword *>(state);
    int total = 0;
    r[41] = reinterpret_cast<Dword>(*output);
    if (!r[2])
        return 0;
    if (int(r[2]) <= requested)
        requested = int(r[2]);
    if (r[1]) {
        int n = int(r[1]);
        if (requested < n)
            n = requested;
        total = n;
        for (int i = 0; i < n; ++i) {
            field<Dword>(reinterpret_cast<void *>(r[41]), 0) =
                field<Dword>(reinterpret_cast<void *>(r[4]), 0);
            r[41] += 4;
            r[4] += 4;
        }
        r[2] -= Dword(n);
        r[1] -= Dword(n);
        requested -= n;
    }
    int full = requested / 28 * 28, remainder = requested % 28;
    r[37] = Dword(full);
    using Decode = void(__cdecl *)(void *);
    if (full > 0) {
        field<Decode>(r, 0)(r + 37);
        r[2] -= Dword(full);
        total += full;
        r[40] += Dword(full / 28) * 15;
    }
    if (remainder > 0) {
        Dword destination = r[41];
        r[41] = reinterpret_cast<Dword>(r + 9);
        r[37] = Dword(remainder);
        field<Decode>(r, 0)(r + 37);
        r[40] += 15;
        r[2] -= Dword(remainder);
        r[4] = r[41] + r[37] * 4;
        r[41] -= 112;
        r[1] = 0 - r[37];
        for (int i = 0; i < remainder; ++i)
            field<Dword>(reinterpret_cast<void *>(destination), i * 4) =
                field<Dword>(reinterpret_cast<void *>(r[41]), i * 4);
        total += remainder;
    }
    if (int(r[2]) < 1)
        r[1] = 0;
    return total;
}

void *__fastcall save_predictor(void *state, void *, void *output) {
    field<Dword>(output, 0) = field<Dword>(state, 152);
    field<Dword>(output, 4) = field<Dword>(state, 156);
    return output;
}

void __fastcall load_predictor(void *state, void *, const void *input) {
    field<Dword>(state, 152) = field<Dword>(input, 0);
    field<Dword>(state, 156) = field<Dword>(input, 4);
}
} // namespace nfs::sound::backend::codecs
