#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::game::runtime::physics::contacts_pending {
using kernel::numeric::Extended;

void accumulate_wheel_damage(void *controller, const float *point, float impulse) {
    if (point[1] > 0.0f)
        return;
    void *body = at<void *>(at<void *>(controller, 28), 1768);
    if (impulse < 3.0f)
        return;
    const auto format = kernel::numeric::current_format();
    const Extended x(point[0], format), z(point[2], format);
    const Extended half_width = Extended(at<float>(body, 120), format) * 0.5f;
    const Extended half_length = Extended(at<float>(body, 128), format) * 0.5f;
    int wheel;
    if (half_length < z) {
        wheel = x < -half_width ? 0 : 1;
        if (!(x < -half_width) && !(half_width < x))
            return;
    } else {
        if (!(z < -half_length))
            return;
        wheel = x < -half_width ? 3 : 2;
        if (!(x < -half_width) && !(half_width < x))
            return;
    }
    const Extended next = Extended(impulse, format) * 0.02f +
                          Extended(at<float>(controller, 12 + wheel * 4), format);
    at<float>(controller, 12 + wheel * 4) = next < Extended(0.0f, format)   ? 0.0f
                                             : Extended(1.0f, format) < next ? 1.0f
                                                                             : float(next);
}

extern const Dword damage_surface_tags[10];

void apply_surface_damage(void *renderable, float *point, float amount) {
    if (!at<Byte>(renderable, 20))
        return;
    const auto format = kernel::numeric::current_format();
    const float scaled_amount = float(Extended(amount, format) * 16.0f);
    point[2] = -point[2];
    auto *record = bytes(at<void *>(renderable, 76));
    for (int index = 0; index < 10; ++index, record += 72) {
        if (point[0] < at<float>(record, 28) || at<float>(record, 40) < point[0] ||
            point[1] < at<float>(record, 32) || at<float>(record, 44) < point[1] ||
            point[2] < at<float>(record, 36) || at<float>(record, 48) < point[2])
            continue;
        const Dword kind = at<Dword>(record, 4);
        at<Dword>(renderable, 52) = at<Dword>(renderable, 48) = kind;
        at<Dword>(renderable, 56) = damage_surface_tags[kind];
        at<Byte>(record, 53) = 1;
        at<Dword>(record, 56) = graphics::unresolved::camera_state;
        const int setting = int(network::packets::rule_value(system(72), 6));
        if ((setting == 2 || network::packets::rule_value(system(72), 6) == 3) &&
            (kind == 2 || kind == 3 || kind == 4 || kind == 5 || kind == 8 || kind == 9))
            continue;
        const Extended next =
            Extended(at<float>(record, 60), format) + Extended(scaled_amount, format);
        at<float>(record, 60) = Extended(at<float>(record, 64), format) < next
                                      ? at<float>(record, 64)
                                      : float(next);
    }
}
} // namespace nfs::game::runtime::physics::contacts_pending
