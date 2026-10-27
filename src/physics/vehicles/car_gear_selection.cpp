#include "car_physics_frame.hpp"
#include "car_drive_frame_x87.hpp"
namespace nfs::game::runtime::physics::car_frame {

void select_automatic_gear_by_force(void *car) {
    const Dword gear = at<Dword>(car, 96);
    int last = int(at<Dword>(car, 780) - 1);
    if (at<Byte>(car, 776)) {
        switch (at<Byte>(car, 1586)) {
        case 0:
        case 1:
        case 2:
            return;
        case 4:
        case 5:
            last = 2;
            break;
        }
    }
    if (int(gear) <= 1)
        return;
    const auto scaled_threshold = [&](float threshold) {
        if (!(drive::absolute(drive::retained(threshold)) < drive::retained(10000)))
            return threshold;
        for (unsigned index = 0; index < 4; ++index) {
            void *wheel = wheel_at(car, index);
            if (at<Byte>(wheel, 221))
                return float((drive::retained(at<float>(car, index < 2 ? 888 : 884)) /
                              drive::retained(at<float>(wheel, 216))) *
                             threshold);
        }
        return 0.f;
    };
    const float upper = scaled_threshold(at<float>(car, 852 + gear * 4));
    const float lower = scaled_threshold(at<float>(car, 848 + gear * 4));
    if (at<int>(car, 1596) < 0)
        ++at<Dword>(car, 1596);
    if (at<int>(car, 1596) > 0)
        --at<Dword>(car, 1596);
    const Byte current = Byte(gear);
    if (upper < at<float>(car, 92)) {
        if (at<int>(car, 1596) < 0)
            return;
        if (at<float>(car, 852 + gear * 4) < at<float>(car, 3168)) {
            at<Byte>(car, 1582) = current;
            if (int(current) < last) {
                at<Byte>(car, 1582) = Byte(current + 1);
                at<Dword>(car, 1596) = 30;
            }
        }
    }
    if (drive::retained(at<float>(car, 92)) < drive::retained(lower) * .9f && at<int>(car, 1596) < 1 &&
        drive::retained(at<float>(car, 3168)) < drive::retained(at<float>(car, 848 + gear * 4)) * .9f) {
        at<Byte>(car, 1582) = current;
        if (current > 2) {
            at<Byte>(car, 1582) = Byte(current - 1);
            at<Dword>(car, 1596) = Dword(-30);
        }
    }
}

void select_reverse_braking(void *car) {
    at<Dword>(car, 1588) &= ~1u;
    if (!at<Byte>(car, 776) || !race_started)
        return;
    bool stationary_brake = false;
    const auto swap_to_reverse = [&]() {
        at<Byte>(car, 1582) = 0;
        at<Byte>(car, 1592) = at<Byte>(car, 1593);
        at<Byte>(car, 1576) = at<Byte>(car, 1581);
        at<Byte>(car, 1581) = 0;
        at<Byte>(car, 1593) = 0;
        at<Byte>(car, 1580) = 0;
    };
    // The saved TEST AH,41 / JP branch accepts ordered equality at 50.
    const auto can_switch = [&]() {
        const auto spin = drive::absolute(drive::retained(at<float>(car, 92)));
        return (spin < drive::retained(50) || float(spin) == 50) &&
               drive::absolute(drive::retained(at<float>(car, 3160))) < drive::retained(1);
    };
    if (!at<Dword>(car, 96)) {
        if (at<Byte>(car, 1581)) {
            swap_to_reverse();
        } else if (at<Byte>(car, 1576)) {
            if (at<float>(car, 3168) > -2.23515f && can_switch()) {
                at<Byte>(car, 1582) = 2;
            } else {
                at<Byte>(car, 1593) = at<Byte>(car, 1592);
                at<Byte>(car, 1581) = at<Byte>(car, 1576);
                at<Byte>(car, 1576) = 0;
                at<Byte>(car, 1592) = 0;
                at<Byte>(car, 1583) = 0;
            }
        } else {
            stationary_brake = true;
        }
    } else if (at<Byte>(car, 1581) && at<float>(car, 3168) < 2.23515f && can_switch()) {
        swap_to_reverse();
    } else if (at<Dword>(car, 96) == 1 && at<Byte>(car, 1576)) {
        at<Byte>(car, 1582) = 2;
    } else {
        stationary_brake = true;
    }
    if (!at<Byte>(car, 776)) {
        if (at<Byte>(car, 1584) > 1)
            select_automatic_gear_by_force(car);
    } else if (!at<Byte>(car, 1582)) {
        at<Byte>(car, 1586) = 1;
    } else if (at<Byte>(car, 1582) > 1) {
        at<Byte>(car, 1586) = 3;
    }
    if (stationary_brake && drive::absolute(drive::retained(at<float>(car, 3168))) < drive::retained(1) &&
        drive::absolute(drive::retained(at<float>(car, 3160))) < drive::retained(1) && !at<Byte>(car, 1576)) {
        at<Dword>(car, 1588) |= 1;
        at<Byte>(car, 1581) = 255;
        at<Byte>(car, 1593) = 1;
        at<Byte>(car, 1580) = 255;
    }
}

void update_automatic_gear_selection(void *car) {
    const Dword previous = at<Dword>(car, 96), selected = at<Byte>(car, 1582);
    if (!race_started) {
        at<Byte>(car, 1582) = 2;
        at<Byte>(car, 1586) = 3;
        return;
    }
    if (selected != previous) {
        at<Dword>(car, 100) = at<Dword>(car, 784);
        at<Dword>(car, 96) = selected;
        at<Byte>(car, 1584) = Byte(selected);
        if (int(previous) < int(selected) && selected > 2)
            at<Dword>(car, 1792) |= 4096;
        announce_gear_selection(0);
    } else if (at<Byte>(car, 1586) == 3 || at<Byte>(car, 1586) == 4) {
        select_automatic_gear_by_force(car);
    }
}

void update_manual_gear_selection(void *car) {
    const Byte selected = at<Byte>(car, 1582);
    const Dword previous = at<Dword>(car, 96);
    if (selected != previous) {
        at<Dword>(car, 100) = at<Dword>(car, 784);
        at<Dword>(car, 96) = selected;
        at<Byte>(car, 1586) = selected;
        at<Byte>(car, 1584) = selected;
        if (int(previous) < int(selected) && selected > 2)
            at<Dword>(car, 1792) |= 4096;
        announce_gear_selection(0);
    }
}
} // namespace nfs::game::runtime::physics::car_frame
