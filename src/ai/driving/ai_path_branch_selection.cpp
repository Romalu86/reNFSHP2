#include "../../police/pursuit/police_chase_x87.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/ai/lifecycle/ai_dependencies.hpp"
#include "ai_path_frame_x87.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

void *construct_path_reference(void *reference, void *group, Dword endpoint) {
    at<void *>(reference, 0) = group;
    at<Dword>(reference, 4) = endpoint;
    at<Dword>(reference, 8) = 0;
    at<std::uint16_t>(reference, 44) = 0;
    return reference;
}

void select_path_section(void *reference, void *section) {
    if (section)
        at<void *>(reference, 8) = section;
}

void update_path_branch(void *path, void *entity) {
    void *section = at<void *>(path, 24);
    if (!section)
        return;
    const auto *last_node = static_cast<const float *>(dependencies::section_last_node(section));
    const float position[3] = {last_node[0], last_node[1], last_node[2]};
    void *group = bytes(system(76)) + 4 + invoke<Dword>(entity, 60 / 4) * 42080;
    path_frame::discard_node_distance(
        static_cast<const float *>(dependencies::path_group_entry(group, at<int>(section, 20))),
        position);
    path_frame::discard_node_distance(
        static_cast<const float *>(dependencies::path_group_entry(group, at<int>(section, 24))),
        position);
    const Dword endpoint = at<Dword>(section, 20 + (at<Dword>(path, 20) == 1 ? 4 : 0));
    void *reference = dependencies::path_group_entry(group, int(endpoint));
    alignas(4) Byte candidates[36];
    at<std::uint16_t>(candidates, 32) = 0;
    for (unsigned index = 0; index < at<std::uint16_t>(reference, 44); ++index) {
        const Dword candidate =
            dependencies::section_candidate(bytes(reference) + 12, std::uint16_t(index));
        if (candidate != reinterpret_cast<Dword>(section))
            dependencies::append_section_candidate(candidates, candidate);
    }
    for (unsigned index = 0; index < at<std::uint16_t>(candidates, 32); ++index)
        if (at<Dword>(path, 36) == at<Dword>(candidates, index * 4))
            return;
    void *next = invoke<void *>(entity, 108 / 4, static_cast<void *>(candidates));
    at<void *>(path, 36) = next;
    if (!next)
        at<void *>(path, 36) = section;
    at<Dword>(path, 32) = 1;
    const Dword next_endpoint = at<Dword>(section, 20 + (at<Dword>(path, 20) == 1 ? 4 : 0));
    next = at<void *>(path, 36);
    if (next_endpoint != at<Dword>(next, 20) && next_endpoint == at<Dword>(next, 24)) {
        at<Dword>(path, 28) = at<Dword>(next, 36) - 1;
        at<Dword>(path, 32) = ~Dword(0);
    } else {
        at<Dword>(path, 28) = 0;
        at<Dword>(path, 32) = 1;
    }
}
// Exhausting either search budget retains the saved success path.
Byte find_pursuit_spawn(void *entity, Byte reverse, float minimum_distance, float player_range,
                        Dword *section_output, int *node_output) {
    return police_frame::find_pursuit_spawn_retained(entity, reverse, minimum_distance, player_range, section_output, node_output);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
