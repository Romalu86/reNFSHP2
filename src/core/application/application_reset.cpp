#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include <intrin.h>
namespace nfs::game::runtime::ai {
// This state is separate from the effects PRNG.
void seed_random(Dword seed) {
    Dword value = seed + 4063039062u;
    random_state[0] = value;
    random_state[1] = value -= 1778116461;
    random_state[2] = value += 1039382086;
    random_state[3] = value += 1088284525;
    random_state[4] = value -= 1758299423;
    random_state[5] = value += 3517578215u;
    random_calls = lifecycle::pending::fractional_random_calls = 0;
}
} // namespace nfs::game::runtime::ai
namespace nfs::application::pending {
// The catch-all protects vehicle-load finalization.
void reset_subsystems(lifecycle::Manager *owner) {
    using namespace game::runtime;
    void *rules = system(72);
    if (!at<Dword>(rules, 140)) {
        const Dword seed = Dword(__rdtsc());
        rules = system(72);
        at<Dword>(rules, 144) = seed;
    }
    ai::seed_random(at<Dword>(rules, 144));
    stepping = 1;
    owner->slots[9]->methods->reset_level(owner->slots[9]);
    try {
        lifecycle::phase_dependencies::finalize_vehicle_load();
    } catch (...) {
        lifecycle::level_load_failed = 1;
    }
    lifecycle::phase_dependencies::finish_level_load();
    for (auto *subsystem : owner->slots)
        if (subsystem)
            subsystem->methods->reset_level(subsystem);
    stepping = 0;
}

void reset_race_state() {
    using namespace game;
    using runtime::at;
    using runtime::system;
    stop_requested = 0;
    if (!at<Dword>(system(112), 68) || at<std::int16_t>(system(104), 10) < 4) {
        frame_clock::elapsed = 0;
        graphics::unresolved::camera_state = 0;
        race_ticks = runtime::started_frames = 0;
        frame_clock::paused = count_race_ticks = runtime::race_started = 0;
        start_deadline = 0;
        frame_clock::step = 1;
    }
}
} // namespace nfs::application::pending
