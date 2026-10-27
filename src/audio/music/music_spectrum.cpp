#include "nfs/audio/music/music_clients.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio::spectrum {

bool power_of_two(Dword samples) {
    return samples >= 2 && !(samples & (samples - 1));
}
// This is the first set bit, rather than a generic logarithm.
Dword index_bits(Dword samples) {
    if (samples < 2) {
        std::fprintf(stderr,
                     ">>> Error in fftmisc.c: argument %d to NumberOfBitsNeeded is too small.\n",
                     samples);
        std::exit(1);
    }
    Dword bit = 0;
    while (!(samples & (Dword(1) << (bit & 31))))
        ++bit;
    return bit;
}

Dword reverse_bits(Dword value, Dword bits) {
    Dword reversed = 0;
    while (bits) {
        reversed = reversed * 2 + (value & 1);
        value >>= 1;
        --bits;
    }
    return reversed;
}

long double frequency(Dword samples, Dword index) {
    if (samples <= index)
        return 0;
    if (index <= samples / 2)
        return static_cast<long double>(index) / samples;
    return -static_cast<long double>(samples - index) / samples;
}
// The argument and sine have separate saved binary32 stores.
long double sine_window(float phase) {
    const float argument = float(static_cast<long double>(phase) *
                                 static_cast<long double>(3141.59265358979291704599745571613311767578125));
    const float sine = float(runtime::actor_sine_retained(argument));
    return static_cast<long double>(sine) * 0.5;
}

void reset(Dword, Dword, int samples) {
    state::spectrum<Dword>(audio_layout::spectrum_random_value) = network::rules::random();
    for (int index = 0; index < samples; ++index)
        state::spectrum<Dword>(audio_layout::spectrum_imaginary_input + index * 4) = 0;
}
// Explicit float stores and retained extended products follow the saved x87
// order.
void transform(Dword samples, int inverse, const float *real_input, const float *imaginary_input,
               float *real, float *imaginary) {
    if (!power_of_two(samples)) {
        std::fprintf(stderr, "Error in fft():  NumSamples=%u is not power of two\n", samples);
        std::exit(1);
    }
    if (!real_input || !real || !imaginary) {
        std::fprintf(stderr, "Error in fft_double():  %s == NULL\n",
                     !real_input ? "RealIn"
                     : !real     ? "RealOut"
                                 : "ImagOut");
        std::exit(1);
    }
    Dword bits = index_bits(samples);
    for (Dword index = 0; index < samples; ++index) {
        Dword output = reverse_bits(index, bits);
        real[output] = real_input[index];
        imaginary[output] = imaginary_input ? imaginary_input[index] : 0;
    }
    auto X = [](float value) { return static_cast<long double>(value); };
    float angle = static_cast<float>(inverse ? -6.28318530717958623199592693708837032318115234375 : 6.28318530717958623199592693708837032318115234375);
    Dword half = 1;
    for (Dword block = 2; block <= samples; block *= 2) {
        const auto delta = X(angle) / static_cast<long double>(block);
        float step = float(delta), twice_negative = float(delta * -2.0f), negative = -step;
        float sine_two = float(runtime::actor_sine_retained(twice_negative));
        float sine_one = float(runtime::actor_sine_retained(negative));
        float cosine_two = float(runtime::actor_cosine_retained(twice_negative));
        float cosine_one = float(runtime::actor_cosine_retained(negative));
        float recurrence = float(X(cosine_one) + cosine_one);
        for (Dword start = 0; start < samples; start += block) {
            auto previous = X(cosine_one);
            float previous_two = cosine_two, imaginary_one = sine_one, imaginary_two = sine_two;
            for (Dword offset = 0; offset < half; ++offset) {
                float real_twiddle = float(previous * recurrence - previous_two);
                previous_two = float(previous);
                previous = X(real_twiddle);
                const auto imaginary_twiddle = X(imaginary_one) * recurrence - imaginary_two;
                imaginary_two = imaginary_one;
                imaginary_one = float(imaginary_twiddle);
                Dword left = start + offset, right = left + half;
                const auto product_real = X(real_twiddle) * real[right] -
                                          imaginary_twiddle * imaginary[right];
                float product_imaginary = float(imaginary_twiddle * real[right] +
                                                X(real_twiddle) * imaginary[right]);
                real[right] = float(X(real[left]) - product_real);
                imaginary[right] = float(X(imaginary[left]) - product_imaginary);
                real[left] = float(product_real + real[left]);
                imaginary[left] = float(X(product_imaginary) + imaginary[left]);
            }
        }
        half = block;
    }
    if (inverse) {
        const auto scale = 1.0L / static_cast<long double>(samples);
        for (Dword index = 0; index < samples; ++index) {
            real[index] = float(scale * real[index]);
            imaginary[index] = float(scale * imaginary[index]);
        }
    }
}

void magnitudes(int *destination, const float *real, const float *imaginary, int samples) {
    for (int index = 0; index < samples; ++index) {
        const auto first = static_cast<long double>(real[index]);
        const auto second = static_cast<long double>(imaginary[index]);
        const float squared = float(second * second + first * first);
        const float magnitude = float(__builtin_sqrtl(static_cast<long double>(squared)));
        destination[index] = runtime::hardware::current_integer_f32(magnitude);
    }
}
} // namespace nfs::game::audio::spectrum
