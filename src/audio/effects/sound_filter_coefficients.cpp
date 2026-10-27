#include "nfs/audio/backend/sound_backend.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend::pending {
extern const float sine_coefficients[6], cosine_coefficients[6], two_pi, pi, window_phase,
    window_base, window_scale;


// The eleventh power has an explicit saved float store.
long double sine_polynomial(float angle) {
    auto X = [&](float value) { return static_cast<long double>(value); };
    auto x = X(angle);
    while (X(two_pi) < x)
        x = x - X(two_pi);
    auto square = x * x, third = square * x, fifth = third * square, seventh = fifth * square,
         ninth = seventh * square;
    float eleventh = float(ninth * square);
    auto result = x - third * sine_coefficients[0];
    result = result + fifth * sine_coefficients[1];
    result = result - seventh * sine_coefficients[2];
    result = result + ninth * sine_coefficients[3];
    result = result - X(eleventh) * sine_coefficients[4];
    return result + (X(eleventh) * square) * sine_coefficients[5];
}
// Cosine polynomial; no host libm substitution.
long double cosine_polynomial(float angle) {
    auto X = [&](float value) { return static_cast<long double>(value); };
    auto x = X(angle);
    while (X(two_pi) < x)
        x = x - X(two_pi);
    auto square = x * x, fourth = square * square, sixth = fourth * square,
         eighth = fourth * fourth;
    auto result = X(1) - square * cosine_coefficients[0];
    result = result + fourth * cosine_coefficients[1];
    result = result - sixth * cosine_coefficients[2];
    result = result + eighth * cosine_coefficients[3];
    result = result - (eighth * square) * cosine_coefficients[4];
    return result + (sixth * sixth) * cosine_coefficients[5];
}
// Five coefficients, saved window and reverse normalization.
void configure_effect(void *state, int type) {

    auto X = [&](float value) { return static_cast<long double>(value); };
    if ((type == 3 || type == 4) && field<float>(state, 52) > 0.8f)
        field<Dword>(state, 52) = 1061997773;
    if (type == 2 || type == 3) {
        float cutoff = field<float>(state, type == 2 ? 56 : 52);
        field<float>(state, 48) = cutoff;
        for (int i = 1; i < 5; ++i) {
            auto angle_value = static_cast<long double>(i) * pi;
            float angle = float(angle_value);
            float input = float(angle_value * cutoff);
            field<float>(state, 48 - i * 4) = float(sine_polynomial(input) / X(angle));
        }
    } else if (type == 4) {
        float high = field<float>(state, 56), low = field<float>(state, 52);
        field<float>(state, 48) = float(X(high) - low);
        for (int i = 1; i < 5; ++i) {
            float angle = float(X(float(i)) * pi);
            float sine = float(sine_polynomial(float((X(high) - low) * angle * 0.5f)));
            auto cosine = cosine_polynomial(float((X(high) + low) * angle * 0.5f));
            field<float>(state, 48 - i * 4) = float((cosine * sine) * (X(2) / X(angle)));
        }
    }
    float total = 0;
    for (int i = 0; i < 5; ++i) {
        if (type == 2 || type == 4) {
            auto value =
                (X(window_base) -
                 cosine_polynomial(float(X(float(i)) * window_phase)) * window_scale) *
                field<float>(state, 32 + i * 4);
            field<float>(state, 32 + i * 4) = float(value);
            total = float(value + total);
        } else if (type == 3) {
            auto value =
                -((X(window_base) -
                   cosine_polynomial(float(X(float(i)) * window_phase)) * window_scale) *
                  field<float>(state, 32 + i * 4));
            float stored = float(value);
            field<float>(state, 32 + i * 4) = stored;
            total = float(i & 1 ? X(total) - stored : X(total) + stored);
        }
    }
    if (type == 3) {
        field<float>(state, 48) = float(X(field<float>(state, 48)) + 1);
        total = float(X(total) + 1);
    }
    auto scale = (X(total) + total) - field<float>(state, 48);
    if (scale < X(0))
        scale = -scale;
    scale = X(1) / scale;
    for (unsigned i = 0; i < 5; ++i)
        field<float>(state, 32 + i * 4) = float(scale * field<float>(state, 32 + i * 4));
}
} // namespace nfs::sound::backend::pending
