#include "nfs/audio/codecs/sound_codecs.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend::codecs {

void skip_bits(void *state, int count) {
    Dword bits = field<volatile Dword>(state, 4);
    int available = field<volatile int>(state, 8);
    bits >>= unsigned(count) & 31;
    available = int(Dword(available) - Dword(count));
    field<volatile int>(state, 8) = available;
    field<volatile Dword>(state, 4) = bits;
    if (available < 8) {
        const Byte *input = field<const Byte *volatile>(state, 0);
        bits |= Dword(*reinterpret_cast<const volatile Byte *>(input)) <<
                (unsigned(available) & 31);
        field<volatile Dword>(state, 4) = bits;
        field<const Byte *volatile>(state, 0) = input + 1;
        field<volatile int>(state, 8) = int(Dword(available) + 8);
    }
}

Dword read_bits(void *state, int count) {
    Dword bits = field<volatile Dword>(state, 4);
    const Dword mask = table<volatile Dword>(7129216 + Dword(count) * 4);
    int available = field<volatile int>(state, 8);
    const Dword value = bits & mask;
    bits >>= unsigned(count) & 31;
    available = int(Dword(available) - Dword(count));
    field<volatile int>(state, 8) = available;
    field<volatile Dword>(state, 4) = bits;
    if (available < 8) {
        const Byte *input = field<const Byte *volatile>(state, 0);
        bits |= Dword(*reinterpret_cast<const volatile Byte *>(input)) <<
                (unsigned(available) & 31);
        field<const Byte *volatile>(state, 0) = input + 1;
        field<volatile Dword>(state, 4) = bits;
        field<volatile int>(state, 8) = int(Dword(available) + 8);
    }
    return value;
}

void decode_excitation(void *state, int mode, float *output) {
    if (!mode) {
        for (int i = 0; i < 108; i += 2) {
            Dword value = field<Dword>(state, 4) & 3;
            if (value == 1) {
                output[i] = -2;
                skip_bits(state, 2);
            } else if (value == 3) {
                output[i] = 2;
                skip_bits(state, 2);
            } else {
                output[i] = 0;
                skip_bits(state, 1);
            }
        }
        return;
    }
    int selector = 0, index = 0;
    while (index < 108) {
        Byte symbol = table<Byte>(7129512 + selector * 256 + (field<Dword>(state, 4) & 255));
        unsigned row = symbol * 12;
        selector = table<int>(7130024 + row);
        skip_bits(state, table<int>(7130028 + row));
        if (symbol < 2) {
            int magnitude = 7;
            while (read_bits(state, 1) == 1)
                magnitude = int(Dword(magnitude) + 1);
            output[index] =
                float(read_bits(state, 1) == 1 ? magnitude : int(0u - Dword(magnitude)));
            index += 2;
        } else if (symbol < 4) {
            int zeros = int(read_bits(state, 6)) + 7;
            if (108 < index + zeros * 2)
                zeros = (108 - index) >> 1;
            for (int i = 0; i < zeros; ++i) {
                output[index] = 0;
                index += 2;
            }
        } else {
            field<Dword>(output, index * 4) = table<Dword>(7130032 + row);
            index += 2;
        }
    }
}

void initialize_speech(const Byte *input, void *state) {
    field<Dword>(state, 4) = *input;
    field<Dword>(state, 8) = 8;
    field<const Byte *>(state, 0) = input + 1;
    read_bits(state, 1);
    field<int>(state, 12) = 32 - int(read_bits(state, 4));
    field<volatile float>(state, 16) = float(static_cast<long double>(read_bits(state, 4) + 1) * 8.f);
    const long double growth =
        static_cast<long double>(read_bits(state, 6)) * speech_growth + speech_bias;
    for (unsigned i = 1; i < 64; ++i)
        field<volatile float>(state, 16 + i * 4) =
            float(growth * field<volatile float>(state, 12 + i * 4));
    for (unsigned i = 0; i < 12; ++i) {
        field<volatile Dword>(state, 272 + i * 4) = 0;
        field<volatile Dword>(state, 320 + i * 4) = 0;
    }
    for (unsigned i = 0; i < 324; ++i)
        field<Dword>(state, 368 + i * 4) = 0;
}

void speech_interpolate(float *output) {
    auto X = [](float value) { return static_cast<long double>(value); };
    for (int i = 0; i < 108; i += 2) {
        auto *p = output + i;
        *p = float(((X(p[-5]) + p[5]) * speech_outer - (X(p[-3]) + p[3]) * speech_middle) +
                   (X(p[-1]) + p[1]) * speech_inner);
    }
}

void reflection_coefficients(const float *reflection, float *output) {
    auto X = [](float value) { return static_cast<long double>(value); };
    float work[24];
    for (int i = 10; i >= 0; --i)
        field<volatile Dword>(work, (i + 1) * 4) = field<volatile Dword>(reflection, i * 4);
    work[0] = 1;
    for (int i = 0; i < 12; ++i) {
        auto value = -(X(work[11]) * reflection[11]);
        for (int j = 10; j >= 0; --j) {
            value = value - X(work[j]) * reflection[j];
            field<volatile float>(work, (j + 1) * 4) = float(value * reflection[j] + work[j]);
        }
        field<volatile float>(work, 0) = float(value);
        field<volatile float>(work, (12 + i) * 4) = float(value);
        for (int j = 0; j < i; ++j)
            value = value - X(output[j]) * work[11 + i - j];
        output[i] = float(value);
    }
}
// Each row preserves the saved twelve-product accumulation order.
void speech_filter(void *state, int first, int groups) {
    auto X = [](float value) { return static_cast<long double>(value); };
    float coefficients[12];
    reflection_coefficients(reinterpret_cast<const float *>(static_cast<Byte *>(state) + 272),
                            coefficients);
    auto *history = reinterpret_cast<float *>(static_cast<Byte *>(state) + 320);
    auto *output = reinterpret_cast<float *>(static_cast<Byte *>(state) + 1664) + first;
    static const Byte order[12][12] = {
        {8, 4, 0, 9, 5, 1, 11, 10, 6, 2, 7, 3}, {8, 4, 0, 9, 5, 1, 10, 6, 2, 11, 7, 3},
        {8, 4, 0, 9, 5, 1, 6, 2, 11, 10, 7, 3}, {8, 4, 0, 5, 1, 9, 6, 2, 11, 10, 7, 3},
        {4, 0, 8, 5, 1, 9, 6, 2, 11, 10, 7, 3}, {4, 0, 8, 5, 1, 9, 6, 2, 11, 10, 3, 7},
        {4, 0, 8, 5, 1, 9, 2, 11, 10, 6, 3, 7}, {4, 0, 8, 1, 9, 5, 2, 11, 10, 6, 3, 7},
        {0, 8, 4, 1, 9, 5, 2, 11, 10, 6, 3, 7}, {0, 8, 4, 1, 9, 5, 2, 11, 10, 6, 7, 3},
        {0, 8, 4, 1, 9, 5, 11, 10, 6, 2, 7, 3}, {0, 8, 4, 9, 5, 1, 11, 10, 6, 2, 7, 3}};
    for (int block = 0; block < groups; ++block) {
        for (unsigned j = 0; j < 12; ++j) {
            unsigned h = order[j][0];
            auto sum = X(coefficients[(h + j) % 12]) * history[h];
            for (unsigned k = 1; k < 12; ++k) {
                h = order[j][k];
                sum += X(coefficients[(h + j) % 12]) * history[h];
            }
            sum = sum + output[j];
            field<volatile float>(history, (11 - j) * 4) = float(sum);
            field<volatile float>(output, j * 4) = float(sum);
        }
        output += 12;
    }
}

void decode_speech_block(void *state) {
    auto X = [](float value) { return static_cast<long double>(value); };
    float delta[12];
    Dword code = read_bits(state, 6);
    bool excitation_mode = int(code) < field<int>(state, 12);
    delta[0] = float((X(table<float>(7129256 + code * 4)) - field<float>(state, 272)) * 0.25f);
    for (unsigned i = 1; i < 4; ++i)
        delta[i] = float((X(table<float>(7129256 + read_bits(state, 6) * 4)) -
                          field<float>(state, 272 + i * 4)) *
                         0.25f);
    for (unsigned i = 4; i < 12; ++i)
        delta[i] = float((X(table<float>(7129320 + read_bits(state, 5) * 4)) -
                          field<float>(state, 272 + i * 4)) *
                         0.25f);
    auto *output = reinterpret_cast<float *>(static_cast<Byte *>(state) + 1664);
    for (int window = 216; window < 648; window += 108) {
        int delay = int(read_bits(state, 8));
        volatile float weight =
            float(static_cast<long double>(read_bits(state, 4)) * speech_history_scale);
        volatile float gain = field<volatile float>(state, 16 + read_bits(state, 6) * 4);
        unsigned parity = read_bits(state, 1), zero = read_bits(state, 1);
        float storage[118];
        float *excitation = storage + 5;
        decode_excitation(state, excitation_mode, excitation + parity);
        if (zero) {
            for (unsigned i = 0; i < 54; ++i)
                excitation[1 - parity + i * 2] = 0;
        } else {
            for (unsigned i = 0; i < 5; ++i)
                storage[i] = excitation[108 + i] = 0;
            speech_interpolate(excitation + 1 - parity);
            gain = float(X(gain) * 0.5f);
        }
        const auto *history =
            reinterpret_cast<const float *>(static_cast<Byte *>(state) + 368) + (window - delay);
        for (unsigned i = 0; i < 108; ++i)
            output[i] = float(X(gain) * excitation[i] + X(weight) * history[i]);
        output += 108;
    }
    for (unsigned i = 0; i < 324; ++i)
        field<Dword>(state, 368 + i * 4) = field<Dword>(state, 2096 + i * 4);
    for (unsigned pass = 0; pass < 4; ++pass) {
        for (unsigned i = 0; i < 12; ++i)
            field<float>(state, 272 + i * 4) =
                float(X(delta[i]) + field<float>(state, 272 + i * 4));
        speech_filter(state, int(pass * 12), pass == 3 ? 33 : 1);
    }
}
} // namespace nfs::sound::backend::codecs
