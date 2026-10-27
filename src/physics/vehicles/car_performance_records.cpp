#include "car_physics_frame.hpp"
namespace nfs::game::runtime::physics::car_frame {
// Receiver is car+71C performance storage.
void record_quarter_mile(void *statistics) {
    if (at<float>(statistics, 24) >= 402.336f &&
        (!at<Dword>(statistics, 48) || at<int>(statistics, 8) < at<int>(statistics, 48))) {
        at<Dword>(statistics, 48) = at<Dword>(statistics, 8);
        at<Dword>(statistics, 56) = at<Dword>(statistics, 28);
    }
}

void record_four_hundred_metres(void *statistics) {
    if (at<float>(statistics, 24) >= 400 &&
        (!at<Dword>(statistics, 52) || at<int>(statistics, 8) < at<int>(statistics, 52))) {
        at<Dword>(statistics, 52) = at<Dword>(statistics, 8);
        at<Dword>(statistics, 60) = at<Dword>(statistics, 28);
    }
}

void record_sixty_mph(void *statistics) {
    if (at<float>(statistics, 28) >= 26.8218f &&
        (!at<Dword>(statistics, 64) || at<int>(statistics, 8) < at<int>(statistics, 64)))
        at<Dword>(statistics, 64) = at<Dword>(statistics, 8);
    if (at<float>(statistics, 28) >= 27.778f &&
        (!at<Dword>(statistics, 72) || at<int>(statistics, 8) < at<int>(statistics, 72)))
        at<Dword>(statistics, 72) = at<Dword>(statistics, 8);
}

void record_hundred_mph(void *statistics) {
    if (at<float>(statistics, 28) >= 44.703f &&
        (!at<Dword>(statistics, 68) || at<int>(statistics, 8) < at<int>(statistics, 68)))
        at<Dword>(statistics, 68) = at<Dword>(statistics, 8);
    if (at<float>(statistics, 28) >= 44.4448f &&
        (!at<Dword>(statistics, 76) || at<int>(statistics, 8) < at<int>(statistics, 76)))
        at<Dword>(statistics, 76) = at<Dword>(statistics, 8);
}

void record_braking_distance(void *statistics, int braking) {
    if (!braking)
        return;
    const auto update = [&](unsigned state, float threshold) {
        switch (at<Dword>(statistics, state)) {
        case 0:
            if (at<float>(statistics, 28) >= threshold)
                at<Dword>(statistics, state) = 1;
            break;
        case 1:
            if (at<float>(statistics, 28) <= threshold) {
                at<Dword>(statistics, state) = 2;
                at<Dword>(statistics, state + 4) = at<Dword>(statistics, 20);
            }
            break;
        case 2:
            if (at<float>(statistics, 28) <= 1.5f) {
                at<Dword>(statistics, state) = 0;
                float distance =
                    float(number(at<float>(statistics, 20)) - at<float>(statistics, state + 4));
                const float previous = at<float>(statistics, state + 8);
                if (previous != 0 && !(distance < previous))
                    distance = previous;
                at<float>(statistics, state + 8) = distance;
            }
            break;
        }
    };
    update(80, 26.8218f);
    update(92, 44.703f);
    update(104, 27.778f);
    update(116, 44.4448f);
}
} // namespace nfs::game::runtime::physics::car_frame
