#include "../../ai/driving/ai_driver_runtime.hpp"
namespace nfs::game::runtime::ai::cars {
// AL indicates whether the three output fields were supplied.
Byte plan_cop_pursuit_route(void *entity, int *direction, float *distance, Dword *branch) {
    void *target = chased_entity(entity);
    if (at<Dword>(entity, 572) != 5 || !target)
        return 0;
    void *current = invoke<void *>(entity, 1);
    const int current_sample = invoke<int>(entity, 2);
    void *destination = invoke<void *>(target, 1);
    const int destination_sample = invoke<int>(target, 2);
    if (!current || current_sample < 0 || !destination || destination_sample < 0)
        return 0;
    void *population = system(76);
    *branch = 0;
    const Dword group = invoke<Dword>(entity, 15);
    void *route = bytes(entity) + 600;
    int endpoint;
    const float *target_position = invoke<const float *>(target, 0);
    const float *position = invoke<const float *>(entity, 0);
    entities::pending::plan_graph_route(bytes(population) + 4 + group * 42080, current,
                                        current_sample, position, destination, destination_sample,
                                        target_position, route, &endpoint, distance);
    if (current == destination) {
        *direction = 1;
        if (*distance < 0.f) {
            *distance = -*distance;
            *direction = -1;
        }
    } else {
        Dword selected = at<Dword>(entity, 240);
        if (endpoint != at<int>(current, 20 + (std::int32_t(selected) > 0 ? 4 : 0)))
            selected = Dword(0) - selected;
        *direction = std::int32_t(selected);
        if (at<std::int16_t>(entity, 1800))
            *branch = at<std::uint16_t>(route, 1200) ? at<Dword>(route, 0) : 0;
        else
            *branch = word_pointer(destination);
    }
    return 1;
}

Dword __fastcall autonomous_cop_choose_branch(void *entity, void *, Dword branches_word) {
    void *branches = reinterpret_cast<void *>(branches_word);
    const Dword count = at<std::uint16_t>(branches, 32);
    if (!count)
        return 0;
    if (count == 1)
        return at<Dword>(branches, 0);
    if (chased_entity(entity)) {
        int direction;
        float distance;
        Dword branch;
        if (plan_cop_pursuit_route(entity, &direction, &distance, &branch)) {
            const Dword endpoint = branch ? at<Dword>(reinterpret_cast<void *>(branch),
                                                    at<int>(entity, 240) == -1 ? 20 : 24)
                                         : Dword(-1);
            void *current = at<void *>(entity, 244);
            const Dword current_endpoint =
                current ? at<Dword>(current, at<Dword>(entity, 240) == 1 ? 24 : 20) : Dword(-1);
            if (current_endpoint != endpoint)
                for (Dword index = 0; index < count; ++index)
                    if ((std::uint16_t(index) < std::uint16_t(count)
                             ? at<Dword>(branches, std::uint16_t(index) * 4)
                             : 0) == branch)
                        return branch;
        }
    }
    return entities::choose_path_branch(entity, nullptr, branches);
}
} // namespace nfs::game::runtime::ai::cars
