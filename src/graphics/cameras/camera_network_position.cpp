#include "camera_controller_runtime.hpp"
#include "../../math/scalar/retained_transcendentals.hpp"
#include <cstring>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Network camera arithmetic requires native x87 intermediates");

namespace nfs::game::world::camera {
void __fastcall advance_network_position(void *behavior, void *, void *state) {
    void *owner = state ? bytes(state) - 4 : nullptr;
    void *director = at<void *>(owner, 4);
    if (!director)
        return;
    const volatile Dword target[3] = {at<volatile Dword>(director, 288),
                                     at<volatile Dword>(director, 292),
                                     at<volatile Dword>(director, 296)};
    at<volatile Dword>(state, 16) = at<volatile Dword>(director, 288);
    const Dword target_y = at<volatile Dword>(director, 292);
    const Dword target_z = at<volatile Dword>(director, 296);
    at<volatile Dword>(state, 20) = target_y;
    at<volatile Dword>(state, 24) = target_z;

    const auto ticks = static_cast<long double>(std::int32_t(kernel::timer_count()));
    const long double phase = ticks * at<float>(behavior, 8) * 60.f /
                              static_cast<long double>(std::int32_t(kernel::timer_frequency));
    const auto remainder = kernel::numeric::hardware::unit_remainder_retained(&phase);
    at<volatile float>(behavior, 4) = float(remainder);
    const float angle = float(remainder);
    float sine, cosine;
    runtime::turn_sine_cosine(&angle, &sine, &cosine);

    const auto horizontal_x = static_cast<long double>(cosine) * at<float>(behavior, 12);
    const auto horizontal_z = static_cast<long double>(sine) * at<float>(behavior, 20);
    const float height = at<float>(behavior, 16);
    const Dword target_words[3] = {target[0], target[1], target[2]};
    float position[3];
    std::memcpy(position, target_words, sizeof(position));
    const float x = float(static_cast<long double>(position[0]) + horizontal_x);
    Dword x_bits;
    std::memcpy(&x_bits, &x, sizeof(x_bits));
    at<volatile Dword>(state, 4) = x_bits;
    const float y = float(static_cast<long double>(position[1]) + height);
    Dword y_bits;
    std::memcpy(&y_bits, &y, sizeof(y_bits));
    at<volatile Dword>(state, 8) = y_bits;
    at<volatile Dword>(state, 32) = 1117126656;
    at<volatile Dword>(state, 28) = 1117126656;
    const float *up = static_cast<const float *volatile &>(up_direction);
    const float z = float(static_cast<long double>(position[2]) + horizontal_z);
    Dword z_bits;
    std::memcpy(&z_bits, &z, sizeof(z_bits));
    at<volatile Dword>(state, 12) = z_bits;

    const Dword up_x = at<Dword>(up, 0);
    const Dword up_y = at<Dword>(up, 4);
    const Dword up_z = at<Dword>(up, 8);
    at<volatile Dword>(state, 60) = up_x;
    at<volatile Dword>(state, 64) = up_y;
    at<volatile Dword>(state, 68) = up_z;
}
} // namespace nfs::game::world::camera
