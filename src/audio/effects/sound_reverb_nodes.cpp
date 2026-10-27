#include "nfs/audio/effects/sound_reverb.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend::pending {
void reset_effect(void *);
void configure_effect(void *, int);
void effect_samples(void *, int, void *, void *);
long double cosine_polynomial(float);
extern const float pi, two_pi, filter_gain_scale;
} // namespace nfs::sound::backend::pending
namespace nfs::sound::backend::effects {

using Node = pending::Processor;

int process_cache(void *node, int count, void *scratch, void *output, Byte) {
    if (field<std::int16_t>(node, 36)) {
        field<std::int16_t>(node, 36) = 0;
        field<std::int16_t>(node, 38) = 1;
    }
    if (field<int>(node, 32) < count) {
        if (auto memory = field<void *>(node, 28))
            release(memory);
        field<void *>(node, 28) = allocate(Dword(count) * 4);
        field<int>(node, 32) = count;
    }
    if (!field<std::int16_t>(node, 38)) {
        copy_memory(output, field<void *>(node, 28), Dword(count) * 4);
        field<std::int16_t>(node, 36) = 1;
        return count;
    }
    void *upstream = field<void *>(node, 8);
    int result = field<Node>(upstream, 0)(upstream, count, output, scratch, 1);
    if (result > 0) {
        copy_memory(output, scratch, Dword(count) * 4);
        copy_memory(field<void *>(node, 28), scratch, Dword(count) * 4);
        field<std::int16_t>(node, 38) = 0;
        return count;
    }
    return result;
}

void release_cache(void *node) {
    if (auto memory = field<void *>(node, 28))
        release(memory);
}

int construct_cache(void *node) {
    for (unsigned i = 8; i <= 20; i += 4)
        field<Dword>(node, i) = 0;
    field<Byte>(node, 27) = 0;
    field<void(__cdecl *)(void *)>(node, 4) = release_cache;
    field<Node>(node, 0) = process_cache;
    field<Dword>(node, 32) = field<Dword>(node, 28) = 0;
    field<std::int16_t>(node, 36) = 0;
    field<std::int16_t>(node, 38) = 1;
    return 0;
}

int process_sum(void *node, int count, void *scratch, void *output, Byte) {
    if (field<int>(node, 32) < count) {
        if (auto memory = field<void *>(node, 28))
            release(memory);
        field<int>(node, 32) = count;
        field<void *>(node, 28) = allocate(Dword(count) * 4);
    }
    void *first = field<void *>(node, 8);
    int result = field<Node>(first, 0)(first, count, scratch, output, 1);
    if (result <= 0)
        return result;
    void *second = field<void *>(node, 12);
    result = field<Node>(second, 0)(second, count, scratch, field<void *>(node, 28), 2);
    if (result <= 0)
        return result;

    for (int i = 0; i < count; ++i)
        field<float>(output, i * 4) =
            float(static_cast<long double>(field<float>(field<void *>(node, 28), i * 4)) +
                  field<float>(output, i * 4));
    return count;
}

void release_sum(void *node) {
    if (auto memory = field<void *>(node, 28))
        release(memory);
}

int construct_sum(void *node) {
    for (unsigned i = 8; i <= 20; i += 4)
        field<Dword>(node, i) = 0;
    field<Byte>(node, 27) = 0;
    field<void(__cdecl *)(void *)>(node, 4) = release_sum;
    field<Node>(node, 0) = process_sum;
    field<Dword>(node, 32) = field<Dword>(node, 28) = 0;
    return 0;
}
// The third and fourth feedback values have explicit float stores.
int process_delay(void *node, int count, void *input, void *output, Byte) {
    if (auto upstream = field<void *>(node, 8)) {
        int result =
            field<Node>(upstream, 0)(upstream, count, output, input, field<Byte>(node, 26));
        if (result <= 0)
            return result;
    }

    auto X = [&](float v) { return static_cast<long double>(v); };
    int remaining = count;
    auto *in = static_cast<float *>(input), *out = static_cast<float *>(output);
    while (remaining > 0) {
        if (field<int>(node, 40) <= field<int>(node, 36))
            field<int>(node, 36) = 0;
        int n = field<int>(node, 40) - field<int>(node, 36);
        if (remaining < n)
            n = remaining;
        auto *history = field<float *>(node, 28) + field<int>(node, 36);
        for (int i = 0; i < n; i += 4) {
            auto first = (X(field<float>(node, 44)) * history[i] + in[i]) + denormal_bias;
            auto second =
                (X(history[i + 1]) * field<float>(node, 44) + in[i + 1]) + denormal_bias;
            float third =
                float((X(field<float>(node, 44)) * history[i + 2] + in[i + 2]) + denormal_bias);
            float fourth =
                float((X(field<float>(node, 44)) * history[i + 3] + in[i + 3]) + denormal_bias);
            out[i] = float(first * field<float>(node, 48) + history[i]);
            out[i + 1] = float(second * field<float>(node, 48) + history[i + 1]);
            out[i + 2] = float(X(third) * field<float>(node, 48) + history[i + 2]);
            out[i + 3] = float(X(fourth) * field<float>(node, 48) + history[i + 3]);
            history[i] = float(first);
            history[i + 2] = third;
            history[i + 1] = float(second);
            history[i + 3] = fourth;
        }
        remaining -= n;
        in += n;
        out += n;
        field<int>(node, 36) += n;
    }
    return count;
}

void release_delay(void *node) {
    if (auto memory = field<void *>(node, 32)) {
        release(memory);
        field<Dword>(node, 32) = 0;
        field<Dword>(node, 28) = 0;
    }
}

int construct_delay(void *node) {
    for (unsigned i = 8; i <= 20; i += 4)
        field<Dword>(node, i) = 0;
    field<Byte>(node, 27) = 0;
    field<void(__cdecl *)(void *)>(node, 4) = release_delay;
    field<Node>(node, 0) = process_delay;
    field<Dword>(node, 32) = field<Dword>(node, 28) = 0;
    return 0;
}

void set_delay(void *node, const int *parameters) {
    release_delay(node);

    auto gain = static_cast<long double>(parameters[0] >> 1) * delay_scale;
    field<float>(node, 44) = float(gain);
    field<float>(node, 48) = float(-gain);
    int count = delay_samples(parameters[1] >> 8, parameters[2] >> 8);
    field<int>(node, 40) = count;
    auto raw = allocate(Dword(count) * 4 + 16);
    field<void *>(node, 32) = raw;
    auto aligned = reinterpret_cast<void *>((reinterpret_cast<Dword>(raw) + 15) & ~Dword(15));
    field<void *>(node, 28) = aligned;
    zero_memory(aligned, Dword(count) * 4);
    field<Dword>(node, 36) = 0;
}

int process_gain(void *node, int count, void *input, void *output, Byte) {
    if (auto upstream = field<void *>(node, 8)) {
        int result =
            field<Node>(upstream, 0)(upstream, count, output, input, field<Byte>(node, 26));
        if (result <= 0)
            return result;
    }

    for (int i = 0; i < count; ++i)
        field<float>(output, i * 4) =
            float(static_cast<long double>(field<float>(input, i * 4)) * field<float>(node, 28));
    return count;
}

int construct_gain(void *node) {
    for (unsigned i = 8; i <= 20; i += 4)
        field<Dword>(node, i) = 0;
    field<Byte>(node, 27) = 0;
    field<Dword>(node, 4) = 0;
    field<Node>(node, 0) = process_gain;
    field<Dword>(node, 28) = 0;
    return 0;
}

void set_gain(void *node, const int *parameters) {
    field<float>(node, 28) =
        float(static_cast<long double>(parameters[0]) *
              pending::filter_gain_scale);
}

void notch_samples(void *node, int count, void *input, void *output) {

    auto X = [&](float v) { return static_cast<long double>(v); };
    if (!(X(field<float>(node, 36)) <
          static_cast<long double>(field<int>(node, 28)) * notch_limit) ||
        !(X(0) < X(field<float>(node, 32))) ||
        !(X(field<float>(node, 32)) < static_cast<long double>(field<int>(node, 28) >> 1))) {
        for (int i = 0; i < count; ++i)
            field<Dword>(output, i * 4) = field<Dword>(input, i * 4);
        return;
    }
    for (int i = 0; i < count; ++i) {
        auto a = (X(field<float>(input, i * 4)) + denormal_bias) * field<float>(node, 48);
        auto b = X(field<float>(node, 40)) * field<float>(node, 44);
        a = a + (b + b) * field<float>(node, 56);
        a = a - (X(field<float>(node, 40)) * field<float>(node, 40)) * field<float>(node, 60);
        field<float>(output, i * 4) = float(a);
        field<Dword>(node, 60) = field<Dword>(node, 56);
        field<Dword>(node, 56) = field<Dword>(output, i * 4);
    }
}

int process_notch(void *node, int count, void *input, void *output, Byte) {
    if (auto upstream = field<void *>(node, 8)) {
        count = field<Node>(upstream, 0)(upstream, count, output, input, field<Byte>(node, 26));
        if (count <= 0)
            return count;
    }
    notch_samples(node, count, input, output);
    return count;
}

int construct_notch(void *node) {
    for (unsigned i = 8; i <= 20; i += 4)
        field<Dword>(node, i) = 0;
    field<Byte>(node, 27) = 0;
    field<Dword>(node, 4) = 0;
    field<Node>(node, 0) = process_notch;
    field<Dword>(node, 52) = field<Dword>(node, 56) = field<Dword>(node, 60) = 0;
    return 0;
}
// Chained polynomial, rather than a host square-root call.
long double root_polynomial(float input) {
    auto x = static_cast<long double>(input);
    long double terms[7] = {x * root_coefficients[0], x, x, x, x, x, x};
    for (unsigned i = 1; i < 7; ++i)
        terms[i] = (x * terms[i - 1]) * root_coefficients[i];
    auto sum = terms[6] + terms[5];
    for (int i = 4; i >= 0; --i)
        sum = sum + terms[i];
    return sum + 1;
}

void set_notch(void *node, const int *parameters) {

    auto X = [&](float v) { return static_cast<long double>(v); };
    float gain = float(static_cast<long double>(parameters[3]) * pending::filter_gain_scale);
    auto center = static_cast<long double>(parameters[0] >> 8),
         width = static_cast<long double>(parameters[2] >> 8);
    float rate = float(static_cast<long double>(parameters[1] >> 8));
    field<float>(node, 32) = float(center);
    field<float>(node, 36) = float(width);
    field<int>(node, 28) = parameters[1] >> 8;
    auto pole = X(1) - (width * pending::pi) / X(rate);
    field<float>(node, 40) = float(pole);
    float square = float(pole * pole);
    float angle = float((center * pending::two_pi) / X(rate));
    auto cosine = pending::cosine_polynomial(angle);
    auto twice = X(field<float>(node, 40)) + field<float>(node, 40);
    auto value = cosine * (twice / (X(square) + 1));
    field<float>(node, 44) = float(value);
    float root_input = float(-(value * value));
    field<float>(node, 48) = float((root_polynomial(root_input) * (X(1) - square)) * gain);
}

int process_bandpass(void *node, int count, void *input, void *output, Byte) {
    if (auto upstream = field<void *>(node, 8)) {
        count = field<Node>(upstream, 0)(upstream, count, output, input, field<Byte>(node, 26));
        if (count <= 0)
            return count;
    }
    pending::effect_samples(static_cast<Byte *>(node) + 28, count, input, output);
    return count;
}

int construct_bandpass(void *node) {
    for (unsigned i = 8; i <= 20; i += 4)
        field<Dword>(node, i) = 0;
    field<Byte>(node, 27) = 0;
    field<Dword>(node, 4) = 0;
    field<Node>(node, 0) = process_bandpass;
    pending::reset_effect(static_cast<Byte *>(node) + 28);
    return 0;
}

void set_bandpass(void *node, const int *parameters) {

    auto denominator = static_cast<long double>(parameters[2] >> 8);
    field<float>(node, 80) = float(static_cast<long double>(parameters[0] >> 7) / denominator);
    field<float>(node, 84) = float(static_cast<long double>(parameters[1] >> 7) / denominator);
    pending::configure_effect(static_cast<Byte *>(node) + 28, 4);
}

int process_highpass(void *node, int count, void *input, void *output, Byte) {
    if (auto upstream = field<void *>(node, 8)) {
        int result =
            field<Node>(upstream, 0)(upstream, count, output, input, field<Byte>(node, 26));
        if (result <= 0)
            return result;
    }
    pending::effect_samples(static_cast<Byte *>(node) + 28, count, input, output);
    return count;
}

int construct_highpass(void *node) {
    for (unsigned i = 8; i <= 20; i += 4)
        field<Dword>(node, i) = 0;
    field<Byte>(node, 27) = 0;
    field<Dword>(node, 4) = 0;
    field<Node>(node, 0) = process_highpass;
    pending::reset_effect(static_cast<Byte *>(node) + 28);
    return 0;
}

void set_highpass(void *node, const int *parameters) {

    auto numerator = static_cast<long double>(parameters[0] >> 8);
    field<float>(node, 84) =
        float((numerator + numerator) / static_cast<long double>(parameters[1] >> 8));
    pending::configure_effect(static_cast<Byte *>(node) + 28, 2);
}
// EAX retains the requested count in the body.
int process_identity(void *node, int count, void *, void *output, Byte) {
    copy_memory(output, field<void *>(node, 28), Dword(count) * 4);
    field<Dword>(node, 28) += Dword(count) * 4;
    return count;
}

void bind_input(void *node, void *input) {
    field<Node>(node, 0) = process_identity;
    field<Dword>(node, 4) = 0;
    field<void *>(node, 28) = input;
}
// The constructor leaves the two callback words untouched.
int construct_identity(void *node) {
    for (unsigned i = 8; i <= 20; i += 4)
        field<Dword>(node, i) = 0;
    field<Byte>(node, 27) = 0;
    field<Dword>(node, 28) = 0;
    return 0;
}

void network_samples(void *state, int count, const float *input, float *output) {

    auto X = [&](float v) { return static_cast<long double>(v); };
    auto *read = field<float *>(state, 12) + field<int>(state, 0),
         *write = field<float *>(state, 12) + field<int>(state, 4);
    int i = 0;
    do {
        auto first = X(input[i]) * field<float>(state, 20);
        auto history = X(field<float>(state, 24)) * field<float>(state, 28);
        first = first + write[-1];
        auto feedback = (first * field<float>(state, 16) + input[i]) + network_bias;
        --read;
        --write;
        output[i] = float(first);
        *read = float(feedback);
        auto gain = X(field<float>(state, 32));
        auto result = history + feedback * gain;
        *read = float(result);
        field<float>(state, 24) = float(result);
        ++i;
    } while (i < count);
}

int connect(void *source, void *destination, int output, int input) {
    if (output == 1) {
        if (input == 1 && !field<Dword>(destination, 8)) {
            field<void *>(source, 16) = destination;
            field<void *>(destination, 8) = source;
            field<Byte>(destination, 26) = 1;
            return 0;
        }
        if (input == 2 && !field<Dword>(destination, 12)) {
            field<void *>(source, 16) = destination;
            field<void *>(destination, 12) = source;
            field<Byte>(destination, 26) = 1;
            return 0;
        }
    } else if (output == 2) {
        if (input == 1 && !field<Dword>(destination, 8)) {
            field<void *>(source, 20) = destination;
            field<void *>(destination, 8) = source;
            field<Byte>(destination, 26) = 2;
            return 0;
        }
        if (input == 2 && !field<Dword>(destination, 12)) {
            field<void *>(source, 20) = destination;
            field<void *>(destination, 12) = source;
            field<Byte>(destination, 26) = 2;
            return 0;
        }
    }
    return -1;
}

int integer_root(int value) {
    int i = 0;
    for (;;) {
        int square = int(Dword(i) * Dword(i));
        ++i;
        if (square == value)
            return i - 1;
        if (square > value)
            return i - 2;
    }
}

int prime_delay(int rate, int milliseconds) {
    int value = int(Dword(rate) * Dword(milliseconds)) / 1000;
    if (value < 3)
        value = 3;
    for (;;) {
        int limit = integer_root(value) + 1;
        bool composite = false;
        for (int divisor = 2; divisor <= limit; ++divisor) {
            if (value % divisor == 0) {
                composite = true;
                break;
            }
            if (divisor == limit)
                return value;
        }
        if (composite)
            ++value;
    }
}

int delay_samples(int rate, int milliseconds) {
    return int(Dword(prime_delay(rate, milliseconds / 16)) << 4);
}
} // namespace nfs::sound::backend::effects
