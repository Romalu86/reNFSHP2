#include "nfs/ai/lifecycle/ai_dependencies.hpp"
namespace nfs::game::runtime::ai::dependencies {

void attach_pursuit_car(Dword *list, Dword *entity) {
    if (static_cast<int>(list[1]) < 24) {
        list[2 + list[1]] = reinterpret_cast<Dword>(entity);
        ++list[1];
    }
}

void synchronize_track_vehicle(void *car) {
    void *body = at<void *>(car, 80);
    if (!body)
        return;
    const Dword position_x = at<Dword>(body, 680);
    const float position_z = -at<float>(body, 688);
    at<Dword>(car, 28) = position_x;
    const Dword position_y = at<Dword>(body, 684);
    at<float>(car, 36) = position_z;
    at<Dword>(car, 32) = position_y;
    body = at<void *>(car, 80);
    const float right_z = -at<float>(body, 700);
    const Dword right_x = at<Dword>(body, 692);
    at<Dword>(car, 40) = right_x;
    const Dword right_y = at<Dword>(body, 696);
    at<float>(car, 48) = right_z;
    const float up_z = -at<float>(body, 712);
    at<Dword>(car, 44) = right_y;
    const Dword up_x = at<Dword>(body, 704);
    at<float>(car, 60) = up_z;
    at<Dword>(car, 52) = up_x;
    const float forward_x = -at<float>(body, 716);
    const Dword up_y = at<Dword>(body, 708);
    at<Dword>(car, 56) = up_y;
    at<float>(car, 64) = forward_x;
    const float forward_y = -at<float>(body, 720);
    const Dword forward_z = at<Dword>(body, 724);
    at<Dword>(car, 72) = forward_z;
    at<float>(car, 68) = forward_y;
}

void reset_path_reservations(void *reservations) {
    Dword remaining = at<Dword>(reservations, 8);
    auto *record = bytes(at<void *>(reservations, 12));
    while (remaining) {
        at<Dword>(record, 28) = at<Dword>(record, 32) = 0;
        record += 40;
        --remaining;
    }
    at<Dword>(reservations, 24) = 0;
}

void release_contact_registry(Dword *registry) {
    for (int index = 0; index < static_cast<int>(registry[2]); ++index)
        if (registry[index])
            registry[index] = 0;
    for (int index = 0; index < static_cast<int>(registry[8]); ++index)
        if (registry[3 + index])
            registry[3 + index] = 0;
    registry[2] = registry[8] = 0;
}

void set_participant_enabled(void *owner, Dword slot, Byte enabled) {
    at<Byte>(owner, 1524 + slot * 988) = enabled;
}

void *section_last_node(void *section) {
    const int count = at<int>(section, 36);
    return count > 0 ? bytes(at<void *>(section, 40)) + (count - 1) * 32 : nullptr;
}

void *section_node(void *section, int index) {
    return bytes(at<void *>(section, 40)) + index * 32;
}

void *path_group_entry(void *group, int index) {
    return bytes(group) + 4 + index * 52;
}

Dword section_candidate(void *candidates, std::uint16_t index) {
    return index < at<std::uint16_t>(candidates, 32) ? at<Dword>(candidates, index * 4) : 0;
}

void append_section_candidate(void *candidates, Dword section) {
    auto &count = at<std::uint16_t>(candidates, 32);
    if (count < 8)
        at<Dword>(candidates, count++ * 4) = section;
}
} // namespace nfs::game::runtime::ai::dependencies
