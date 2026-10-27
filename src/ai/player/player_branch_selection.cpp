#include "../driving/ai_driver_runtime.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::cars {

static kernel::numeric::Extended player_branch_cost(void *entity, void *branch) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    if (!branch)
        return E(-1);
    const float base = float(Extended::integer(at<int>(branch, 32), format));
    float occupancy = 0.f;
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if (!at<Dword>(body, 732)) {
        body = invoke<void *>(entities::interface_at(entity, 3), 0);
        if (body && at<Byte>(body, 728)) {
            const int count = at<int>(entity, 8);
            for (int index = 0; index < count; ++index) {
                void *other = at<void *>(entity, 12 + Dword(index) * 4);
                if (!at<Byte>(other, 212))
                    continue;
                void *driver = invoke<void *>(entities::interface_at(other, 2), 2);
                if (driver) {
                    void *other_body = invoke<void *>(entities::interface_at(driver, 3), 1);
                    if (at<Dword>(other_body, 732))
                        continue;
                    other_body = invoke<void *>(entities::interface_at(driver, 3), 0);
                    if (!other_body || !at<Byte>(other_body, 728))
                        continue;
                    if (lifecycle::path_current_section(bytes(driver) + 236) == branch ||
                        lifecycle::path_current_section(bytes(driver) + 248) == branch)
                        occupancy = float(E(occupancy) + 1.f);
                }
                void *spike = spike_entity(other);
                if (spike && invoke<void *>(spike, 1) == branch)
                    occupancy = float(E(occupancy) + 5.f);
                driver = invoke<void *>(entities::interface_at(other, 2), 4);
                if (driver && at<void *>(entities::interface_at(driver, 4), 8) &&
                    (lifecycle::path_current_section(bytes(driver) + 236) == branch ||
                     lifecycle::path_current_section(bytes(driver) + 248) == branch))
                    occupancy = float(E(occupancy) + 2.f);
                void *traffic = traffic_entity(other);
                if (traffic && at<Dword>(traffic, 240) != at<Dword>(entity, 240) &&
                    (lifecycle::path_current_section(bytes(traffic) + 236) == branch ||
                     lifecycle::path_current_section(bytes(traffic) + 248) == branch))
                    occupancy = float(E(occupancy) + 0.5f);
            }
            const auto cost = E(occupancy) + base;
            return E(0) < cost ? cost : E(0);
        }
    }
    return E(base);
}
// Eight saved score slots; the unchecked saved count is retained.
Dword __fastcall player_choose_branch(void *entity, void *, Dword branches_word) {
    void *branches = reinterpret_cast<void *>(branches_word);
    const auto count = at<std::uint16_t>(branches, 32);
    if (!count)
        return 0;
    if (count == 1)
        return at<Dword>(branches, 0);
    void *current = at<void *>(entity, 244);
    const Dword endpoint =
        current ? at<Dword>(current, 20 + (at<int>(entity, 240) == 1 ? 4 : 0)) : Dword(-1);
    void *owner = system(76);
    int skill = at<std::uint16_t>(entities::interface_at(entity, 5), 0);
    if (skill <= at<int>(owner, 84504))
        skill = at<int>(owner, 84504);
    int limit = std::int32_t(at<Dword>(owner, 84500) * Dword(skill)) / 10;
    if (limit <= 1)
        limit = 1;
    const long double random = lifecycle::random_fraction();
    if (random < 0.125L)
        limit = std::int32_t(Dword(limit) - 2);
    else if (random < 0.375L)
        limit = std::int32_t(Dword(limit) - 1);
    if (limit < 1)
        limit = 1;
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if ((!body || !at<Byte>(body, 728)) && !physics::split_screen())
        limit = 10;
    float scores[8] = {};
    for (int index = 0; index < count; ++index) {
        void *branch = at<void *>(branches, Dword(std::uint16_t(index)) * 4);
        if (branch && at<Dword>(branch, 20) == endpoint && at<int>(branch, 32) <= 10)
            scores[index] = float(player_branch_cost(entity, branch));
        else
            scores[index] = -1.f;
    }
    int selected = -1;
    for (int index = 0; index < count; ++index) {
        const auto format = kernel::numeric::current_format();
        if (scores[index] < 0.f || std::isnan(scores[index]) ||
            kernel::numeric::Extended::integer(limit, format) <
                kernel::numeric::Extended(scores[index], format))
            continue;
        if (selected < 0 || scores[index] > scores[selected])
            selected = index;
    }
    if (selected < 0) {
        for (int index = 0; index < count; ++index) {
            if (scores[index] < 0.f)
                continue;
            if (selected < 0 || scores[index] < scores[selected])
                selected = index;
        }
        if (selected < 0)
            selected = 0;
    }
    return std::uint16_t(selected) < count ? at<Dword>(branches, Dword(std::uint16_t(selected)) * 4)
                                           : 0;
}
} // namespace nfs::game::runtime::ai::cars
