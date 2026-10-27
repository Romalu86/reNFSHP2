#include "../../ai/driving/ai_car_behavior.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::cars {

void record_car_controls(void *recorder, Dword player, const Dword *controls) {
    void *records = at<void *>(recorder, 4);
    if (!records)
        return;
    const Dword position = at<volatile Dword>(recorder, 40);
    ++at<volatile Dword>(recorder, 44);
    if (position < 21600) {
        const Dword slot = position * 16;
        at<Dword>(records, slot) = controls[0];
        at<Dword>(records, slot + 4) = controls[1];
        at<Byte>(at<void *>(recorder, 4), at<Dword>(recorder, 40) * 16 + 8) =
            at<Byte>(system(36), 48 + player * 4);
        void *record = bytes(at<void *>(recorder, 4)) + at<Dword>(recorder, 40) * 16;
        if (!at<Byte>(system(36), 64 + player))
            at<Dword>(record, 0) &= 4294967294u;
        else
            at<Dword>(record, 0) |= 1;
        void *car = physics::local_car(system(68), 0);
        if (car && at<void *>(car, 1768))
            at<Dword>(record, 12) = at<Dword>(at<void *>(car, 1768), 680);
        ++at<Dword>(recorder, 40);
    }
    if (at<Dword>(recorder, 40) >= 21600) {
        if (at<Dword>(recorder, 52)) {
            const Dword count = at<Dword>(recorder, 44);
            at<volatile Dword>(recorder, 48) = count;
            at<volatile Dword>(recorder, 40) = 0;
            return;
        }
        const Dword count = at<Dword>(recorder, 44) - 1;
        at<Dword>(recorder, 40) = 21599;
        at<Dword>(recorder, 44) = count;
        at<Dword>(recorder, 48) = count;
    }
}

Byte replay_car_controls(void *recorder, Dword player, Dword *controls) {
    if (at<Dword>(recorder, 44) >= at<Dword>(recorder, 48))
        return 0;
    void *records = at<void *>(recorder, 4);
    if (!records)
        return 0;
    Dword slot = at<Dword>(recorder, 40);
    if (slot >= 21600)
        return 0;
    void *car = physics::local_car(system(68), 0);
    if (car && at<void *>(car, 1768)) {
        const float recorded = at<float>(records, slot * 16 + 12);
        const float current = at<float>(at<void *>(car, 1768), 680);
        // TEST AH,44 / JNP accepts ordered equality; unordered values reject the record.
        if (recorded != current)
            return 0;
    }
    controls[0] = at<Dword>(records, slot * 16);
    controls[1] = at<Dword>(records, slot * 16 + 4);
    ++at<Dword>(recorder, 44);
    ++slot;
    at<Dword>(recorder, 40) = slot;
    if (slot < 21600) {
        if (physics::split_screen())
            ++slot;
        if (std::int32_t(slot) >= at<int>(recorder, 48))
            return 1;
    } else {
        slot = 0;
        at<Dword>(recorder, 40) = 0;
        if (!at<Dword>(recorder, 52))
            return 0;
        if (physics::split_screen())
            slot = 1;
    }
    at<Dword>(recorder, 64 + player * 4) = at<Byte>(at<void *>(recorder, 4), slot * 16 + 8);
    at<Dword>(recorder, 56 + player * 4) = at<Dword>(at<void *>(recorder, 4), slot * 16) & 1;
    return 1;
}

void collect_car_controls(void *input, Dword player, Dword *controls) {
    read_car_action_bindings(input, player, controls);
}

void apply_car_path_controls(void *entity, Dword controls_word) {
    void *car = at<void *>(entity, 368);
    if (!car)
        return;
    const Dword player = at<Dword>(car, 1776);
    auto *controls = reinterpret_cast<Dword *>(controls_word);
    if (at<Dword>(system(72), 140)) {
        if (!replay_car_controls(system(132), player, controls))
            at<Dword>(system(96), 36) = 23;
    } else {
        collect_car_controls(system(100), player, controls);
        record_car_controls(system(132), player, controls);
    }
}
} // namespace nfs::game::runtime::ai::cars
