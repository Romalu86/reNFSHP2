#include "car_physics_frame.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
namespace nfs::game::runtime::physics::car_frame {
namespace {
long double hardware_number(float value) { return static_cast<long double>(value); }
long double hardware_magnitude(long double value) { return value < 0 ? -value : value; }
long double hardware_sine(float turns) { return turn_sine(turns); }
long double hardware_cosine(float turns) { return turn_cosine(turns); }
} // namespace

void solve_wheel_suspension(void *car) {
    const unsigned front_right_contact = at<Byte>(car, 2500);
    const float mass = *reinterpret_cast<const volatile float *>(bytes(car) + 520);
    const unsigned rear_right_contact = at<Byte>(car, 2796);
    const unsigned rear_left_contact = at<Byte>(car, 3092);
    const unsigned front_left_contact = at<Byte>(car, 2204);
    const unsigned contacts = front_left_contact + front_right_contact +
                              rear_right_contact + rear_left_contact;
    at<Dword>(car, 2184) = 0;
    at<Dword>(car, 2176) = 0;
    const auto clear_force = [&](unsigned offset) {
        at<Dword>(car, offset - 4) = 0;
        at<Dword>(car, offset) = 0;
        at<Dword>(car, offset + 4) = 0;
    };
    if (contacts < 2) {
        at<Dword>(car, 2180) = 0;
        clear_force(2476);
        clear_force(3068);
        clear_force(2772);
        return;
    }
    const float initial = float(hardware_number(mass) / static_cast<long double>(contacts));
    at<float>(car, 2180) = initial;
    at<float>(car, 2476) = initial;
    at<Dword>(car, 2472) = 0;
    at<Dword>(car, 2480) = 0;
    at<float>(car, 3068) = initial;
    at<Dword>(car, 3064) = 0;
    at<Dword>(car, 3072) = 0;
    at<float>(car, 2772) = initial;
    at<Dword>(car, 2768) = 0;
    at<Dword>(car, 2776) = 0;
    const float height = float(hardware_number(at<float>(car, 544)) * .9090909f);
    const float rear_length = at<float>(car, 116), front_length = at<float>(car, 112);
    const float front_track = at<float>(car, 532), rear_track = at<float>(car, 536);
    const float bank = at<float>(car, 460);
    const auto retained_mass_length =
        hardware_number(mass) * .5f / hardware_number(at<float>(car, 528));
    const float mass_length = float(retained_mass_length);
    const float front_mass = float(retained_mass_length * rear_length);
    at<float>(car, 2180) = front_mass;
    at<float>(car, 2476) = front_mass;
    const float rear_mass = float(hardware_number(mass_length) * front_length);
    at<float>(car, 3068) = rear_mass;
    at<float>(car, 2772) = rear_mass;
    const float cos_zero = float(hardware_cosine(0));
    const auto sin_zero = hardware_sine(0);
    const float pitch = float(hardware_number(bank) * 3.1415927410125732421875f);
    const float front_delta =
        float((hardware_number(rear_length) * (hardware_number(cos_zero) - 1.f) - sin_zero * height) * mass_length);
    const float rear_delta =
        float(((hardware_number(cos_zero) - 1.f) * front_length + sin_zero * height) * mass_length);
    const auto retained_front_roll =
        ((hardware_number(at<float>(car, 2180)) + front_mass) * .5f) *
        (hardware_number(height) / front_track) * pitch;
    const float front_roll = float(retained_front_roll);
    const float negative_front_roll = float(-retained_front_roll);
    const auto retained_rear_roll =
        ((hardware_number(at<float>(car, 3068)) + rear_mass) * .5f) *
        (hardware_number(height) / rear_track) * pitch;
    const float rear_roll = float(retained_rear_roll);
    const float negative_rear_roll = float(-retained_rear_roll);
    const auto add = [&](unsigned offset, long double value) {
        at<float>(car, offset) = float(value + at<float>(car, offset));
    };
    const auto finish_rear = [&]() {
        add(3068, hardware_number(rear_roll) + rear_delta);
        add(2772, hardware_number(negative_rear_roll) + rear_delta);
    };
    if (contacts == 4) {
        add(2180, hardware_number(front_roll) + front_delta);
        add(2476, hardware_number(negative_front_roll) + front_delta);
        finish_rear();
    } else if (contacts == 3) {
        if (!at<Byte>(car, 2204)) {
            at<float>(car, 2476) = float(hardware_number(front_delta) + front_delta +
                                          at<float>(car, 2180) + at<float>(car, 2476));
            clear_force(2180);
            finish_rear();
        } else if (at<Byte>(car, 2500)) {
            add(2180, hardware_number(front_roll) + front_delta);
            add(2476, hardware_number(negative_front_roll) + front_delta);
            if (!at<Byte>(car, 3092)) {
                at<float>(car, 2772) = float(hardware_number(rear_delta) + rear_delta +
                                              at<float>(car, 3068) + at<float>(car, 2772));
                clear_force(3068);
            } else {
                at<float>(car, 3068) = float(hardware_number(rear_delta) + rear_delta +
                                              at<float>(car, 2772) + at<float>(car, 3068));
                clear_force(2772);
            }
        } else {
            at<float>(car, 2180) = float(hardware_number(front_delta) + front_delta +
                                          at<float>(car, 2180) + at<float>(car, 2476));
            clear_force(2476);
            finish_rear();
        }
    } else if (contacts == 2) {
        const bool front_left = at<Byte>(car, 2204) != 0, rear_left = at<Byte>(car, 3092) != 0;
        const bool front_right = at<Byte>(car, 2500) != 0, rear_right = at<Byte>(car, 2796) != 0;
        if (!front_left && !front_right) {
            at<float>(car, 3068) =
                float(hardware_number(at<float>(car, 2180)) + rear_roll + at<float>(car, 3068));
            at<float>(car, 2772) =
                float(hardware_number(at<float>(car, 2476)) + negative_rear_roll + at<float>(car, 2772));
            clear_force(2180);
            clear_force(2476);
        } else if (!rear_left && !rear_right) {
            at<float>(car, 2180) =
                float(hardware_number(at<float>(car, 3068)) + front_roll + at<float>(car, 2180));
            at<float>(car, 2476) =
                float(hardware_number(at<float>(car, 2772)) + negative_front_roll + at<float>(car, 2476));
            clear_force(3068);
            clear_force(2772);
        } else if (!front_left && !rear_left) {
            at<float>(car, 2476) = float(hardware_number(front_delta) + front_delta +
                                          at<float>(car, 2180) + at<float>(car, 2476));
            at<float>(car, 2772) = float(hardware_number(rear_delta) + rear_delta + at<float>(car, 3068) +
                                          at<float>(car, 2772));
            clear_force(2180);
            clear_force(3068);
        } else if (!front_right && !rear_right) {
            at<float>(car, 2180) = float(hardware_number(front_delta) + front_delta +
                                          at<float>(car, 2476) + at<float>(car, 2180));
            at<float>(car, 3068) = float(hardware_number(rear_delta) + rear_delta + at<float>(car, 2772) +
                                          at<float>(car, 3068));
            clear_force(2476);
            clear_force(2772);
        } else if (!front_left && !rear_right) {
            at<float>(car, 2476) = float(hardware_number(front_delta) + front_delta +
                                          at<float>(car, 2180) + at<float>(car, 2476));
            at<float>(car, 3068) = float(hardware_number(rear_delta) + rear_delta + at<float>(car, 2772) +
                                          at<float>(car, 3068));
            clear_force(2180);
            clear_force(2772);
        } else if (!front_right && !rear_left) {
            at<float>(car, 2180) = float(hardware_number(front_delta) + front_delta +
                                          at<float>(car, 2476) + at<float>(car, 2180));
            at<float>(car, 2772) = float(hardware_number(rear_delta) + rear_delta + at<float>(car, 3068) +
                                          at<float>(car, 2772));
            clear_force(2476);
            clear_force(3068);
        }
    }
    const float speed = at<float>(car, 3132);
    auto centripetal =
        hardware_magnitude((hardware_number(speed) / hardware_number(at<float>(car, 1804))) * hardware_sine(bank) * speed);
    if (centripetal < hardware_number(0))
        centripetal = hardware_number(0);
    else if (hardware_number(10) < centripetal)
        centripetal = hardware_number(10);
    const auto acceleration = centripetal + gravity;
    for (unsigned offset : {2180u, 2476u, 3068u, 2772u})
        at<float>(car, offset) = float(hardware_number(at<float>(car, offset)) * acceleration);
    for (unsigned offset : {2180u, 2476u, 3068u, 2772u}) {
        auto *force = reinterpret_cast<volatile float *>(bytes(car) + offset);
        const float value = *force;
        *force = value > 0 ? value : 0;
    }
}
} // namespace nfs::game::runtime::physics::car_frame
