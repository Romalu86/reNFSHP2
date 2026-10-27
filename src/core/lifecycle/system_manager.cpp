#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/system_tables.hpp"
#include "nfs/core/lifecycle/system_destruction.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"

namespace nfs::lifecycle {
namespace {
using construction::Fields;

void set_methods(Fields *object, const SystemMethods &methods) {
    *reinterpret_cast<const SystemMethods **>(object) = &methods;
}

// Each of the 36 allocations has a placement-delete action. Completed earlier slots are retained
// when a later constructor throws.
template <class Constructor>
System *construct_system(int bytes, const char *label, Dword &pool, Constructor constructor) {
    auto *object = static_cast<Fields *>(allocate(bytes, label, pool));
    if (!object)
        return nullptr;
    try {
        return reinterpret_cast<System *>(constructor(object));
    } catch (...) {
        game::runtime::track::pending::release_named(object, label, pool);
        throw;
    }
}

Fields *construct_track(Fields *object) {
    using namespace construction;
    set_methods(object, base_system_methods);
    unsigned completed = 0;
    try {
        objects::track_state(object + 19);
        completed = 1;
        construct_array(object + 42, 168, 128, objects::track_item, objects::track_item_destroy);
        completed = 2;
        set_methods(object, track_methods);
    } catch (...) {

        if (completed >= 2)
            objects::destroy_array(object + 42, 168, 128, objects::track_item_destroy);
        if (completed >= 1)
            destruction::track_state(object + 19);
        set_methods(object, base_system_methods);
        throw;
    }
    return object;
}

Fields *construct_skid_system(Fields *object) {
    set_methods(object, base_system_methods);
    bool state_constructed = false;
    try {
        construction::render_state(object + 5);
        state_constructed = true;
        set_methods(object, skidsystem_methods);
    } catch (...) {

        if (state_constructed)
            construction::destruction::render_state_member(object + 5);
        set_methods(object, base_system_methods);
        throw;
    }
    return object;
}

Fields *construct_frontend(Fields *object) {
    auto *screen_name = reinterpret_cast<gui::String *>(object + 4);
    auto *previous_screen = reinterpret_cast<gui::String *>(object + 14);
    auto *viewport = reinterpret_cast<gui::engine::Rectangle *>(object + 21);
    set_methods(object, base_system_methods);
    // These stages represent the 43..49 member-unwind states, not game state.
    unsigned stage = 0;
    try {
        object[1] = 0;
        object[2] = 0;
        reinterpret_cast<std::uint8_t *>(object)[12] = 0;
        screen_name->methods = gui::base_methods;
        stage = 1;
        gui::string_construct(screen_name);
        stage = 2;
        object[10] = 0;
        object[12] = 0;
        object[13] = 0;
        previous_screen->methods = gui::base_methods;
        stage = 3;
        gui::string_construct(previous_screen);
        gui::string_copy(previous_screen, &construction::unresolved::shared_string);
        stage = 4;
        object[19] = 1;
        object[20] = 0;
        viewport->methods = gui::base_methods;
        stage = 5;

        gui::engine::construct_rectangle(viewport);
        stage = 6;
        reinterpret_cast<std::uint8_t *>(object)[104] = 0;
        set_methods(object, frontend_methods);
    } catch (...) {
        if (stage >= 6)
            gui::engine::destroy_rectangle(viewport);
        else if (stage == 5)
            viewport->methods = gui::base_methods;
        if (stage >= 4)
            gui::string_destruct(previous_screen);
        else if (stage == 3)
            previous_screen->methods = gui::base_methods;
        if (stage >= 2)
            gui::string_destruct(screen_name);
        else if (stage == 1)
            screen_name->methods = gui::base_methods;
        set_methods(object, base_system_methods);
        throw;
    }
    return object;
}
} // namespace

// Allocation order differs from slot/init order; slot22 stays null.
Manager *construct_manager(Manager *result) {
    using namespace construction;
    kernel::memory_zero(result, 148);
    result->slots[2] = construct_system(4, "DedicatedConsole", ui_pool, [](Fields *object) {
        set_methods(object, base_system_methods);
        set_methods(object, dedicatedconsole_methods);
        return object;
    });
    result->slots[3] = construct_system(8, "NetCryptSystem", level_pool, net_crypt);
    result->slots[5] = construct_system(12, "FontsManager", ui_pool, fonts_manager);
    result->slots[13] = construct_system(81316, "VehicleManager", level_pool, vehicle_manager);
    result->slots[10] = construct_system(152, "TrackManager", track_pool, track_manager);
    result->slots[15] = construct_system(4128, "CEventManager", level_pool, event_manager);
    result->slots[6] = construct_system(188, "LevelManager", level_pool, level_manager);
    result->slots[9] = construct_system(21756, "Track", track_pool, construct_track);
    result->slots[14] = construct_system(160, "WorldLightManager", level_pool, world_light);
    result->slots[8] = construct_system(600, "EnviroReflection", cars_pool, environment);
    result->slots[19] = construct_system(84636, "AI::System", level_pool, ai_system);
    result->slots[18] = construct_system(103016, "GameRules", level_pool, game_rules);
    result->slots[17] = construct_system(1876, "CarManager", cars_pool, car_manager);
    result->slots[20] = construct_system(16, "Audio", audio_pool, [](Fields *object) {
        set_methods(object, base_system_methods);
        object[2] = 0;
        set_methods(object, audio_methods);
        return object;
    });
    result->slots[21] = construct_system(96, "Speech", audio_pool, speech);
    result->slots[7] = construct_system(4, "AudioLoader", audio_pool, [](Fields *object) {
        set_methods(object, base_system_methods);
        set_methods(object, audioloader_methods);
        return object;
    });
    result->slots[16] = construct_system(16, "ParticleSystem", level_pool, particle_system);
    result->slots[23] = construct_system(12, "RaceManager", level_pool, [](Fields *object) {
        set_methods(object, base_system_methods);
        set_methods(object, racemanager_methods);
        object[1] = 0;
        object[2] = 0;
        return object;
    });
    result->slots[11] = construct_system(25024, "Skeleton System", level_pool, skeleton_system);
    result->slots[0] = construct_system(1056, "CRC System", level_pool, [](Fields *object) {
        return crc_system(object, 32, 79764919, 4294967295u, 4294967295u, 1);
    });
    result->slots[25] = construct_system(1268, "Input System", level_pool, input_system);
    result->slots[31] = construct_system(192, "Shadow Manager", level_pool, shadow_manager);
    result->slots[4] = construct_system(8, "Platform System", level_pool, platform_system);
    result->slots[1] = construct_system(8324, "Path System", level_pool, [](Fields *object) {
        set_methods(object, base_system_methods);
        set_methods(object, path_system_methods);
        return object;
    });
    result->slots[26] = construct_system(172, "Network game", level_pool,
                                         [](Fields *object) { return network_game(object, 1); });
    result->slots[27] = construct_system(400, "Server", level_pool,
                                         [](Fields *object) { return network_server(object, 1); });
    result->slots[28] = construct_system(2140, "Client", level_pool, network_client);
    result->slots[29] = construct_system(148, "SkidSystem", level_pool, construct_skid_system);
    result->slots[30] = construct_system(4, "CameraSystem", level_pool, [](Fields *object) {
        set_methods(object, base_system_methods);
        set_methods(object, camerasystem_methods);
        return object;
    });
    result->slots[12] = construct_system(112, "Character System", level_pool, character_system);
    result->slots[32] = construct_system(116, "FogSystem", level_pool, fog_system);
    result->slots[24] = construct_system(108, "FrontEnd", ui_pool, construct_frontend);
    result->slots[33] = construct_system(72, "Replay", replay_pool, replay_system);
    result->slots[34] = construct_system(8, "StartLine", level_pool, start_line);
    result->slots[35] = construct_system(56, "LOD", level_pool, lod_system);
    result->slots[36] = construct_system(300, "FFB", level_pool, feedback_system);
    return result;
}
} // namespace nfs::lifecycle
