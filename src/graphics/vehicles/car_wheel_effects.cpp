#include "car_wheel_effects_runtime.hpp"
#include <cstring>
namespace nfs::game::runtime::physics::contacts_pending {

void update_contact_particles(void *car) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto I = [&](int value) { return Extended::integer(value, format); };
    auto setting = [&](unsigned va) {
        return at<int>(services::pending::rumble_values, va - 6899704);
    };
    auto gain = [&](unsigned va) {
        return at<float>(services::pending::rumble_values, va - 6899704);
    };
    auto integer = [](Extended value) { return int(Dword(value.truncate_to_integer())); };
    auto material_for = [&](int wheel) -> Byte {
        const Dword start = Dword(wheel) * 296;
        return at<Dword>(car, 2020 + start) ? at<Byte>(car, 2074 + start) & 31 : 0;
    };
    Dword first_feedback = 0, second_feedback = 0;
    for (int index = 0; index < 4; ++index) {
        const Dword wheel = Dword(index) * 296;
        void *properties = pending::surface_properties(material_for(index));
        float speed = at<float>(car, 3132);
        if (speed < 0)
            speed = -speed;
        at<Byte>(car, 1716 + Dword(index)) = 0;
        int feedback = 0;
        const bool grounded = at<Byte>(car, wheel + 2204) != 0;
        if (!grounded ||
            E(at<float>(at<void *>(car, 1768), 112)) < Extended::from_double(.9, format)) {
            particles::lifecycle::release_wheel_skid(car, index);
            at<Dword>(car, 1700 + Dword(index) * 4) = 0;
        } else if (wheel_is_punctured(bytes(car) + 7952, Byte(index))) {
            particles::lifecycle::release_wheel_skid(car, index);
            const Dword kind =
                at<Dword>(pending::surface_properties(material_for(index)), 0);
            float slip = float(wheel_slip_strength(car, index));
            bool sparks = false;
            switch (kind) {
            case 0:
            case 5:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 13:
            case 14:
            case 15:
            case 19:
            case 20:
            case 21:
            case 22:
            case 29:
            case 30:
                sparks = speed > 4.470300197601318359375f || slip != 0;
                break;
            }
            if (sparks) {
                float rotation[9], point[3], velocity[3];
                wheel_effect_pose(car, index, rotation, point, velocity);
                auto candidate = E(speed) * 0.074566207826137542724609375f;
                if (!(candidate < E(1)))
                    candidate = E(1);
                if (!(candidate < E(slip)))
                    slip = float(candidate);
                at<float>(car, 1700 + Dword(index) * 4) = slip;
                const float normal[3] = {0, 1, 0};
                emit_sparks(car, point, slip, normal);
                feedback = setting(6899716);
            } else {
                at<Dword>(car, 1700 + Dword(index) * 4) = 0;
                if (speed > gain(6899704)) {
                    const int base = setting(6899716) / 2;
                    feedback = int(Dword(base) + Dword(integer(E(gain(6899708)) * speed * I(base))));
                }
            }
        } else {
            float rotation[9], point[3], velocity[3];
            wheel_effect_pose(car, index, rotation, point, velocity);
            const auto slip = wheel_slip_strength(car, index);
            at<float>(car, 1700 + Dword(index) * 4) = float(slip);
            const float skid_threshold = at<float>(system(140), 8);
            const bool skid = skid_threshold < 1 && E(skid_threshold) < slip;
            if (skid) {
                update_wheel_skid(car, index);
                feedback = setting(6899720);
            } else {
                particles::lifecycle::release_wheel_skid(car, index);
            }
            const float threshold = at<float>(system(140), 16);
            const float stored_slip = at<float>(car, 1700 + Dword(index) * 4);
            if (threshold < 1 && stored_slip > threshold) {
                feedback = int(Dword(feedback) + at<Dword>(properties, 12));
                const auto amount = (E(stored_slip) - threshold) / (E(1) - threshold);
                if (at<Dword>(properties, 32)) {
                    const float strength = amount < E(1) ? float(amount) : 1.f;
                    emit_wheel_particles(car, index, point, at<int>(properties, 32), strength);
                }
            } else {
                feedback = int(Dword(feedback) + at<Dword>(properties, 16));
                if (speed > gain(6899704)) {
                    const auto extra = E(gain(skid ? 6899712 : 6899708)) * speed * I(feedback);
                    feedback = int(Dword(feedback) + Dword(integer(extra)));
                } else
                    feedback = 0;
            }
            if (at<Dword>(properties, 36) && speed > at<float>(properties, 40)) {
                const float strength = float((E(speed) - at<float>(properties, 40)) /
                                             E(at<float>(properties, 44)));
                emit_wheel_particles(car, index, point, at<int>(properties, 36), strength);
            }
        }
        if (index == 0 || index == 3)
            first_feedback += Dword(feedback);
        else
            second_feedback += Dword(feedback);
    }
    if (int(first_feedback) > 65535)
        first_feedback = 65535;
    if (int(second_feedback) > 65535)
        second_feedback = 65535;
    accumulate_rumble(car, int(first_feedback), int(second_feedback), 54);
    const Byte front_second = material_for(1), front_first = material_for(0);
    update_axle_surface_audio(car, 1, front_first, front_second);
    const Byte rear_first = material_for(2), rear_second = material_for(3);
    update_axle_surface_audio(car, 0, rear_second, rear_first);
}
} // namespace nfs::game::runtime::physics::contacts_pending
