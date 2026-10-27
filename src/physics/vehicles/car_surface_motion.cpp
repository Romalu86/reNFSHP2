#include "car_physics_frame.hpp"
#include "car_drive_frame_x87.hpp"
namespace nfs::game::runtime::physics::car_frame {
extern const Byte surface_perturbation[256];  

void update_wheel_surface_motion(void *car) {
    for (unsigned index = 0; index < 4; ++index) {
        void *wheel = wheel_at(car, index);
        if (!at<Byte>(wheel, 256)) {
            at<Dword>(wheel, 260) = 0;
            continue;
        }
        if (contacts_pending::wheel_is_punctured(bytes(car) + 7952, Byte(index))) {
            const float phase = float(drive::integer(at<int>(wheel, 12)) * .125f +
                                      drive::retained(at<float>(wheel, 264)) * 0.15915493667125701904296875f);
            at<float>(wheel, 260) =
                float((drive::retained(1) - drive::absolute(drive::cosine(phase)) * .5f) * at<float>(wheel, 216) * .3f);
            continue;
        }
        const Dword material = !at<Dword>(wheel, 72) ? 0 : at<Byte>(wheel, 126) & 31;
        void *surface = pending::surface_properties(int(material));
        unsigned side = index > 1 ? 1 : 0;
        if (index == 0 || index == 3)
            side += 128;
        const float amplitude = at<float>(surface, 24), frequency = at<float>(surface, 28);
        if (frequency == 0) {
            at<Dword>(wheel, 260) = 0;
            continue;
        }
        const float speed = at<float>(car, 3168), radius = at<float>(wheel, 212);
        const float phase = float(drive::retained(at<float>(car, 1840)) * frequency);
        const Dword phase_index = Dword(hardware::current_integer_f32(phase));
        const Byte sample = surface_perturbation[(phase_index + side) & 255];
        const float noise = float(drive::integer(sample) * .0078125f - 1.f);
        auto displacement = drive::retained(noise) * amplitude * drive::sine(phase);
        const float updated =
            float(drive::retained(speed) * frequency * 6.283185482025146484375f * displacement * 0.0333333350718021392822265625f +
                  at<float>(wheel, 260));
        at<float>(wheel, 260) = updated;
        displacement = drive::absolute(displacement);
        const float cap = float(drive::retained(radius) * .25f);
        if (!(displacement < drive::retained(cap)))
            displacement = drive::retained(cap);
        if (drive::retained(updated) < -displacement)
            at<float>(wheel, 260) = float(-displacement);
        else if (displacement < drive::retained(updated))
            at<float>(wheel, 260) = float(displacement);
        else
            at<float>(wheel, 260) = updated;
    }
}
} // namespace nfs::game::runtime::physics::car_frame
