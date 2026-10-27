#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../math/precision/retained_particle.hpp"

namespace nfs::game::runtime::particles::lifecycle {
namespace {
__declspec(noinline) Dword captured_clock_argument(const void *value) {
    return *static_cast<const volatile Dword *>(value);
}
}

void advance_emitter_clock(void *clock, void *emitter, float elapsed) {
    auto retained = [](float value) { return static_cast<long double>(value); };
    // Inactive clocks never perform a floating load of the elapsed argument.
    const Dword elapsed_word = captured_clock_argument(&elapsed);
    auto elapsed_value = [&] { return retained(at<const volatile float>(&elapsed_word, 0)); };
    void *type = at<void *>(clock, 20);
    switch (at<int>(type, 0)) {
    case 1: {
        if (!at<Byte>(clock, 16))
            return;
        const long double time = elapsed_value() + at<const volatile float>(clock, 8);
        at<volatile float>(clock, 8) = float(time);
        if (time >= at<const volatile float>(clock, 12)) {
            at<volatile Byte>(clock, 16) = 0;
            return;
        }
        break;
    }
    case 2: {
        const long double time = elapsed_value() + at<const volatile float>(clock, 8);
        // The alternating clock compares its retained sum before the FST32.
        const bool expired = time >= at<const volatile float>(clock, 12);
        at<volatile float>(clock, 8) = float(time);
        if (expired) {
            const Byte active = at<Byte>(clock, 16) == 0;
            at<volatile float>(clock, 8) =
                float(time - at<const volatile float>(clock, 12));
            at<volatile Byte>(clock, 16) = active;
            const unsigned period = active ? 4 : 16;
            const Dword value = emitter_random_word() & 65535;
            at<volatile float>(clock, 12) =
                float(static_cast<long double>(value) * 0.0000152587890625f *
                          at<const volatile float>(type, period + 8) +
                      at<const volatile float>(type, period));
        }
        if (!at<Byte>(clock, 16))
            return;
        break;
    }
    }
    const long double total = elapsed_value() + at<const volatile float>(clock, 0);
    at<volatile float>(clock, 0) = float(total);
    if (!(total > at<const volatile float>(clock, 4)))
        return;
    float velocity[3];
    scale_vectors(1, at<const float *>(emitter, 36), at<float>(type, 100), velocity);
    long double period;
    do {
        const float age = float(retained(at<const volatile float>(clock, 0)) -
                                at<const volatile float>(clock, 4));
        at<volatile float>(clock, 0) = age;
        pending::emit_type_particle(type, emitter, velocity, age);
        period = retained_emitter_period(static_cast<Dword *>(type));
        at<volatile float>(clock, 4) = float(period);
    } while (period < at<const volatile float>(clock, 0));
}
} // namespace nfs::game::runtime::particles::lifecycle
namespace nfs::game::runtime::pending {
void advance_emitter_clock_entry(void *clock, void *emitter, float elapsed) {
    particles::lifecycle::advance_emitter_clock(clock, emitter, elapsed);
}
} // namespace nfs::game::runtime::pending
