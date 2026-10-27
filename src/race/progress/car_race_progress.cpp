#include "../../vehicles/frame/car_frame_runtime.hpp"
#include "../rules/race_rules_x87.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::cars {
using namespace physics::car_frame;
namespace {
long double number(float value) { return static_cast<long double>(value); }
}
// Callers use only the low byte of the result.
Byte sample_race_progress(void *progress, void *section, int sample, const float *position,
                          float *output) {
    float begin = at<float>(progress, (at<Dword>(section, 20) + 1) * 52);
    float end = at<float>(progress, (at<Dword>(section, 24) + 1) * 52);
    if (begin == -1 || end == -1) {
        *output = -1;
        return 0;
    }
    void *node = dependencies::section_node(section, sample);
    const float preceding = at<float>(node, 28);
    float pose[21], relative[3];
    dependencies::sample_section_pose(section, sample, pose);
    for (unsigned axis = 0; axis < 3; ++axis)
        relative[axis] = float(number(position[axis]) - pose[axis]);
    const auto projected = runtime::hardware::dot_product(relative, pose + 3) + preceding;
    const float section_length = at<float>(section, 44);
    const auto fraction = section_length != 0 ? projected / number(section_length) : number(0);
    if (!track_is_circuit()) {
        *output = float((number(end) - begin) * fraction + begin);
        return 1;
    }
    const float length = at<float>(progress, 16864);
    if (!(number(begin) < number(length) * .35f) || !(number(length) * .65f < number(end))) {
        if (begin < end)
            begin = float(number(end) + 5.f);
    } else
        end = float(number(end) - length);
    const auto distance = (number(end) - begin) * fraction + begin;
    *output = float(distance);
    // The saved comparison uses the retained value, before the output's float rounding.
    if (distance <= 0.f)
        *output = float(distance + length);
    return 1;
}
// Byte1 of the packet is unwritten and copied as representation.
void send_car_lap_progress(void *client, Dword slot, Dword lap, Dword race_tick) {
    if (!at<Dword>(client, 68))
        return;
    void *car = at<void *>(system(68), 20 + slot * 4);
    if (!car || physics::car_is_remote(car))
        return;
    alignas(4) Byte packet[24];
    at<Dword>(packet, 20) = lap;
    at<Dword>(packet, 4) = slot;
    at<std::uint16_t>(packet, 2) = at<std::uint16_t>(client, 376);
    packet[0] = 32;
    at<Dword>(packet, 8) = race_tick;
    at<Dword>(packet, 12) = at<Dword>(car, 1852);
    at<Dword>(packet, 16) = at<Dword>(car, 1856);
    network::send_packet(word_pointer(bytes(client) + 72), word_pointer(packet), 24, 0);
}

void update_racer_progress(void *entity) {
    if (!track_is_circuit() && at<int>(entity, 568) >= 2)
        return;
    void *progress = bytes(system(76)) + invoke<Dword>(entity, 15) * 42080 + 4;
    if (at<float>(progress, 16864) == -1) {
        at<float>(entity, 564) = -1000000;
        return;
    }
    void *cursor = bytes(entity) + 236;
    void *section = lifecycle::path_current_section(cursor);
    const int sample = entities::path_sample_index(cursor);
    if (!section || sample < 0)
        return;
    const float previous = at<float>(entity, 564);
    float *distance = vector_at(entity, 564);
    const float *position = invoke<const float *>(entity, 0);
    if (!sample_race_progress(progress, section, sample, position, distance) || previous == -1)
        return;
    const auto notify_lap = [&]() {
        const Dword lap = at<Dword>(entity, 568) + 1;
        at<Dword>(entity, 568) = lap;
        void *car = at<void *>(entity, 368);
        if (!car || physics::car_is_remote(car))
            return;
        void *race = at<void *>(system(92), 4);
        const Dword tick = invoke<Dword>(race, 22);
        race = at<void *>(system(92), 4);
        using LapNotification = void(__thiscall *)(void *, Dword, Dword, Dword);
        const auto notify = (*reinterpret_cast<const LapNotification **>(race))[30];
        const Dword slot = network::world::vehicle_slot(entity);
        notify(race, slot, lap, tick);
        const Dword current_lap = at<Dword>(entity, 568);
        void *client = network::client();
        const Dword current_slot = network::world::vehicle_slot(entity);
        send_car_lap_progress(client, current_slot, current_lap, tick);
    };
    if (!track_is_circuit()) {
        if (*distance <= 0)
            notify_lap();
        return;
    }
    const float early = float(number(at<float>(progress, 16864)) * .1f);
    if (previous < early && number(at<float>(progress, 16864)) * .9f < number(*distance)) {
        notify_lap();
        if (at<int>(entity, 572) < at<int>(entity, 568))
            at<Dword>(entity, 572) = at<Dword>(entity, 568);
    } else if (number(at<float>(progress, 16864)) * .9f < number(previous) && *distance < early) {
        --at<Dword>(entity, 568);
    }
}
} // namespace nfs::game::runtime::ai::cars
