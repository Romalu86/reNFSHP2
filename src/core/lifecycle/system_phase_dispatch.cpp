#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"

namespace nfs::lifecycle {
std::uint8_t level_load_failed{};

namespace phase_dependencies {
 
void finish_level_load() {
}

 
void set_track_phase(int) {
}
} // namespace phase_dependencies


void __fastcall load_level(Manager *owner) {
    game::runtime::physics::reset_vehicle_registry();
    game::world::dependency::legacy_clock();
    game::audio::vehicle::Progress progress;
    game::audio::vehicle::construct_progress(&progress, 4);
    Dword completed_at[37];

    // This exception path does not unload any system.
    try {
        for (int index = 0; index < 37; ++index) {
            System *system = owner->slots[index];
            if (system)
                system->methods->load_level(system);
            completed_at[index] = game::world::dependency::legacy_clock();
            if (index == 10 || index == 19 || index == 17 || index == 20)
                game::audio::vehicle::advance_progress(&progress);
        }
    } catch (...) {
        game::audio::vehicle::destroy_progress(&progress);
        throw;
    }
    game::audio::vehicle::destroy_progress(&progress);
    phase_dependencies::report_memory_usage();


    try {
        phase_dependencies::finalize_vehicle_load();
    } catch (...) {
        level_load_failed = 1;
    }
    phase_dependencies::finish_level_load();
}


void __fastcall unload_level(Manager *owner) {
    game::runtime::particles::lifecycle::pending::diagnostic_message("start level deinit\n");
    game::runtime::clear_step_lists();
    for (int index = 37; index;) {
        System *system = owner->slots[--index];
        if (system)
            system->methods->unload_level(system);
    }
    game::runtime::particles::lifecycle::pending::diagnostic_message("end level deinit\n");
}


void __fastcall enter_frontend(Manager *owner) {
    files::archives::clear();
    ini::flush_cache();

    reinterpret_cast<std::uint8_t *>(manager->slots[13])[81296] = 1;
    phase_dependencies::set_track_phase(1);
    game::runtime::physics::reset_vehicle_registry();
    for (int index = 0; index < 37; ++index) {
        System *system = owner->slots[index];
        if (system)
            system->methods->enter_frontend(system);
    }
}


void __fastcall leave_frontend(Manager *owner) {
    files::archives::clear();
    ini::flush_cache();
    reinterpret_cast<std::uint8_t *>(manager->slots[13])[81296] = 0;
    for (int index = 0; index < 37; ++index) {
        System *system = owner->slots[index];
        if (system)
            system->methods->leave_frontend(system);
    }
}
} // namespace nfs::lifecycle
