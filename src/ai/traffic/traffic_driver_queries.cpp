#include "../driving/ai_drive_x87.hpp"
#include "../driving/ai_driver_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {
// Predicate result is AL.
Byte traffic_yields_to_chase(void *entity) {
    const Dword deadline = at<Dword>(entity, 584);
    if (std::int32_t(graphics::unresolved::camera_state) < std::int32_t(deadline) &&
        deadline)
        return 1;
    for (int index = 0; index < at<int>(entity, 8); ++index) {
        void *other = at<void *>(entity, 12 + Dword(index) * 4);
        void *player = invoke<void *>(entities::interface_at(other, 2), 1);
        if (player && target_on_neighboring_section(entity, player)) {
            void *remote = invoke<void *>(entities::interface_at(player, 2), 4);
            if (remote && at<void *>(entities::interface_at(remote, 4), 8) &&
                ai::pair_distance(entity, remote) < 50.f)
                return 1;
        }
    }
    return 0;
}
// The first player farther than50 ends the saved scan.
Byte traffic_vehicle_ahead(void *entity) {
    return drive_frame::traffic_ahead(entity);
}

Dword __fastcall traffic_choose_branch(void *entity, void *, Dword branches_word) {
    void *branches = reinterpret_cast<void *>(branches_word);
    const Dword count = at<std::uint16_t>(branches, 32);
    if (!count)
        return 0;
    if (count == 1)
        return at<Dword>(branches, 0);
    void *current = at<void *>(entity, 244);
    const Dword endpoint =
        current ? at<Dword>(current, at<Dword>(entity, 240) == 1 ? 24 : 20) : Dword(-1);
    const Dword endpoint_offset = at<int>(entity, 240) <= 0 ? 24 : 20;
    void *least_used = nullptr, *single_use = nullptr;
    for (Dword index = 0; index < count; ++index) {
        void *candidate = std::uint16_t(index) < std::uint16_t(count)
                              ? at<void *>(branches, std::uint16_t(index) * 4)
                              : nullptr;
        if (endpoint != at<Dword>(candidate, endpoint_offset))
            continue;
        if (!least_used || at<int>(candidate, 32) < at<int>(least_used, 32))
            least_used = candidate;
        if (at<Dword>(candidate, 32) == 1)
            single_use = candidate;
    }
    if (single_use)
        return word_pointer(single_use);
    if (least_used)
        return word_pointer(least_used);
    return count ? at<Dword>(branches, 0) : 0;
}
} // namespace nfs::game::runtime::ai::cars
