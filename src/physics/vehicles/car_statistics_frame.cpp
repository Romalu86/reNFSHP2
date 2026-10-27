#include "car_physics_frame.hpp"
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::game::runtime::physics::car_frame {

void update_car_statistics(void *car) {
    const int ticks = int(game::race_ticks);
    void *entity = at<void *>(car, 7988);
    bool active = true;
    if (entity) {
        void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
        active = body && at<Byte>(body, 728);
    }
    const auto speed = magnitude(number(at<float>(car, 3168)));
    at<float>(car, 1848) = float(speed);
    at<float>(car, 1840) = float(speed * 0.0333333350718021392822265625f + at<float>(car, 1840));
    if (ticks > 0 && !invoke<Byte>(entity, 47) && !invoke<Byte>(entity, 48)) {
        bool update_average = !network::session_active();
        if (!update_average) {
            void *race = at<void *>(system(92), 4);
            update_average = race && invoke<Byte>(race, 29, at<Dword>(car, 1772));
        }
        if (update_average)
            at<float>(car, 1856) =
                float(number(at<float>(car, 1840)) / (integer(ticks) * 0.01666666753590106964111328125f));
    }
    if (!active)
        return;
    at<float>(car, 1844) =
        float(number(at<float>(car, 1848)) * 0.0333333350718021392822265625f + at<float>(car, 1844));
    entity = at<void *>(car, 7988);
    if (entity && invoke<Dword>(entities::interface_at(entity, 1), 0) == 6)
        return;
    if (!(at<float>(car, 1852) > at<float>(car, 3132)))
        at<float>(car, 1852) = at<float>(car, 3132);
    if (!at<Byte>(car, 1584) || !race_started)
        return;
    if (at<float>(car, 1848) < .5f) {
        at<Dword>(car, 1824) = Dword(ticks);
        at<Dword>(car, 1828) = 0;
        at<Dword>(car, 1844) = 0;
        at<Dword>(car, 1936) = 0;
        at<Dword>(car, 1924) = 0;
        at<Dword>(car, 1912) = 0;
        at<Dword>(car, 1900) = 0;
    }
    at<Dword>(car, 1828) = Dword(ticks) - at<Dword>(car, 1824);
    void *statistics = bytes(car) + 1820;
    record_sixty_mph(statistics);
    record_hundred_mph(statistics);
    record_quarter_mile(statistics);
    record_four_hundred_metres(statistics);
    record_braking_distance(statistics, at<Byte>(car, 1580));
}
// Truncate the retained candidate RPM before converting it to angular velocity.
void update_kinematic_engine_rpm(void *car) {
    const float speed = float(magnitude(number(at<float>(car, 3168))));
    auto rpm = number(0);
    if (at<float>(car, 3168) < 0 && speed > .44703f) {
        at<Byte>(car, 1584) = 0;
        rpm = number(at<float>(car, 184)) * speed;
        const float maximum = float(integer(at<int>(car, 660)) * .85f);
        if (number(maximum) < rpm)
            rpm = number(maximum);
    } else {
        const float idle = float(integer(at<int>(car, 656)));
        if (!(number(at<float>(car, 284)) * idle < number(speed))) {
            at<Byte>(car, 1584) = 2;
            rpm = number(at<float>(car, 192)) * speed;
        } else {
            int gear = 2;
            const int last = int(at<Dword>(car, 780) - 1);
            const float maximum = float(integer(at<int>(car, 660)) * .85f);
            if (last > 2) {
                do {
                    rpm = number(at<float>(car, 184 + Dword(gear) * 4)) * speed;
                    if (!(number(maximum) < rpm) && number(idle) < rpm)
                        break;
                    ++gear;
                } while (gear < last);
            }
            if (gear == last && number(maximum) < rpm)
                rpm = number(maximum);
            at<Byte>(car, 1584) = Byte(gear);
        }
    }
    const Byte gear = at<Byte>(car, 1584);
    at<Byte>(car, 1582) = gear;
    at<Dword>(car, 96) = gear;
    at<float>(car, 4) = float(integer(truncate(rpm)) * 0.10471975803375244140625f);
}
} // namespace nfs::game::runtime::physics::car_frame
