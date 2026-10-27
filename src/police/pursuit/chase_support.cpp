#include "chase_runtime.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

Dword support_statistic_limit(void *settings, Dword kind) {
    return kind < 10 ? at<Dword>(settings, 2268 + kind * 4) : 0;
}

Byte increment_support_statistic(void *record, int kind) {
    if (kind < 0 || kind >= 10)
        return 0;
    const Dword value = at<Dword>(record, 892 + Dword(kind) * 4) + 1;
    if (int(value) > int(support_statistic_limit(system(72), Dword(kind))))
        return 0;
    at<Dword>(record, 892 + Dword(kind) * 4) = value;
    return 1;
}

Byte record_support_request(void *race, Dword participant, int kind) {
    const Dword offset = participant * 988;
    ++at<Dword>(race, offset + 1516);
    return increment_support_statistic(bytes(race) + offset + 584, kind);
}

Byte attach_chase_cop(void *chase, void *cop) {
    if (application::pending::traffic_actor_view(cop))
        ++at<std::uint16_t>(chase, 10);
    const Dword count = at<Dword>(chase, 40);
    if (count >= 3)
        return 0;
    at<void *>(chase, 28 + count * 4) = cop;
    const Dword next_count = at<Dword>(chase, 40) + 1;
    using ActivateMethod = void(__thiscall *)(void *, Dword);
    const auto *methods = *reinterpret_cast<const ActivateMethod **>(cop);
    at<Dword>(chase, 40) = next_count;
    methods[6](cop, 1);
    invoke(perp_interface(cop), 1, chase);
    return 1;
}

Byte request_chase_cop(void *owner, void *chase, Dword reason) {
    const Dword count = at<std::uint16_t>(owner, 80);
    for (Dword index = 0; index < count; ++index) {
        void *cop = at<void *>(owner, index * 4);
        if (at<Dword>(cop, 572))
            continue;
        void *target = at<void *>(chase, 44);
        at<Dword>(cop, 580) = reason;
        if (target) {
            void *session = system(92);
            const Dword participant = network::world::vehicle_slot(target);
            record_support_request(at<void *>(session, 4), participant, 3);
        }
        attach_chase_cop(chase, cop);
        return 1;
    }
    return 0;
}

void position_chase_cops(void *chase, Dword first, int last) {
    for (Dword index = first; int(index) < last; ++index) {
        void *target = index == first ? at<void *>(chase, 44)
                       : index - 1 < at<Dword>(chase, 40)
                           ? at<void *>(chase, 28 + (index - 1) * 4)
                           : nullptr;
        Dword route;
        int node;
        if (!find_pursuit_spawn(target, 1, index == first ? 200.f : 5.f,
                                index == first ? 200.f : 100.f, &route, &node))
            return;
        float position[3], rotation[9];
        sample_route_pose(reinterpret_cast<void *>(route), node, position, rotation, 0);
        void *cop = index < at<Dword>(chase, 40) ? at<void *>(chase, 28 + index * 4) : nullptr;
        position_entity(cop, position, rotation, 0);
        set_car_speed(at<void *>(cop, 368), saved_float(1110626271));
        at<float>(cop, 392) = 1.f;
    }
}

Byte __fastcall request_backup_car(void *chase, void *) {
    at<Byte>(chase, 108) = 1;
    const Byte result = request_chase_cop(&state<Dword>(7353160), chase, 7);
    if (result)
        position_chase_cops(chase, at<Dword>(chase, 40) - 1, at<int>(chase, 40));
    return result;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
