#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include <cstdlib>

namespace nfs::game::runtime::ai::lifecycle {
namespace {
void clear_manager_fields(Dword *owner) {
    for (int group = 0; group < 2; ++group)
        nfs::lifecycle::construction::objects::ai_group_reset(bytes(owner) + 4 + group * 42080);
    at<Dword>(owner, 84164) = at<Dword>(owner, 84168) = at<Dword>(owner, 84172) = 0;
    for (int i = 0; i < 8; ++i)
        at<Dword>(owner, 84176 + i * 4) = 0;
    for (int i = 0; i < 61; ++i)
        at<Dword>(owner, 84392 + i * 4) = 0;
}
} // namespace

void initialize(Dword *owner) {
    pending::read_police_chase_tuning();
    clear_manager_fields(owner);
    if (!state<Dword *>(7353312)) {
        auto *list = static_cast<Dword *>(
            nfs::lifecycle::allocate(10012, "AI::Entity::List", nfs::lifecycle::level_pool));
        state<Dword *>(7353312) = list ? construct_entity_list(list) : nullptr;
    }
    network::world::vehicle_registry = state<Dword *>(7353312);
    pending::load_racer_profiles(&state<Dword>(7352968));
    auto *profile = static_cast<Dword *>(
        nfs::lifecycle::allocate(192, "AI::AINetCarProto", nfs::lifecycle::level_pool));
    if (profile) {
        profile[0] = reinterpret_cast<Dword>(pending::net_profile_base_offsets);
        profile[46] = reinterpret_cast<Dword>(pending::profile_base_methods);
        pending::construct_net_profile(profile, 0);
        const int base_offset = pending::net_profile_base_offsets[1];
        at<const Dword *>(profile, base_offset) = pending::net_profile_methods;
        at<int>(profile, base_offset - 4) = base_offset - 184;
        profile[44] = 6;
    }
    if (profile) {
        auto *interface = bytes(profile) + pending::net_profile_base_offsets[1];
        if (interface && state<Dword>(7353148) < 45)
            state<void *>(7352968 + state<Dword>(7353148)++ * 4) = interface;
    }
    state<Byte>(7352412) = pending::disable_ai != 0;
    if (static_cast<int>(pending::explicit_car_kind) > 0)
        at<Dword>(system(72), 212) = std::atoi(pending::car_kind_text) == 0;
    const char *skin = pending::explicit_car_skin ? pending::car_skin_text : nullptr;
    int variant = 0;
    const bool change_variant = pending::explicit_car_variant != 0;
    if (change_variant)
        variant = pending::parse_car_kind(pending::car_variant_text);
    if (change_variant || skin) {
        Dword index = 0;
        while (index < state<Dword>(7353148)) {
            auto *profile_interface = state<Dword *>(7352968 + index * 4);
            if (void *settings = invoke<void *>(profile_interface, 3)) {
                if (change_variant)
                    at<Dword>(settings, 152) = pending::parse_car_variant(variant);
                if (skin)
                    at<Dword>(settings, 148) = pending::parse_car_skin(skin);
            }
            ++index;
        }
    }
    begin_path_level();
}

void shutdown(Dword *owner) {
    clear_manager_fields(owner);
    Dword *list = state<Dword *>(7353312);
    if (list) {
        destroy_entity_list(list);
        gui::application_release(list);
    }
    state<Dword *>(7353312) = nullptr;
    for (Dword i = 0; i < state<Dword>(7353148); ++i)
        if (Dword *profile = state<Dword *>(7352968 + i * 4))
            invoke(profile, 1, Dword{1});
    state<Dword>(7353148) = 0;
    network::world::vehicle_registry = nullptr;
}

void unload_level(Dword *owner) {
    pending::population_loading = 1;
    clear_pursuit_schedule(reinterpret_cast<Dword *>(bytes(owner) + 84344));
    pending::release_path_population(ai::pending::path_controller);
    clear_entity_list(network::world::vehicle_registry);
    clear_manager_fields(owner);
    end_path_level();
    state<Byte>(7353316) = 0;
}

void reset_level(Dword *owner) {
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, network::world::vehicle_registry);
    for (Dword entity = network::world::first(&iterator); entity;
         entity = network::world::next(&iterator))
        invoke(reinterpret_cast<void *>(entity), 12);
    pending::anchor_paths(system(76));
    pending::reset_path_controller(ai::pending::path_controller);
    // The scheduler at owner+148F0 is distinct from the path controller.
    pending::recount_entity_kinds(reinterpret_cast<Dword *>(bytes(owner) + 84208));
    ai::reset_pursuit_clock(reinterpret_cast<Dword *>(bytes(owner) + 84344));
    ai::reset_pair_clock(state<Dword *>(7353312));
    ai::assign_targets(owner, 1);
    state<Byte>(7353316) = 0;
}
} // namespace nfs::game::runtime::ai::lifecycle
