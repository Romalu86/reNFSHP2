#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "nfs/network/session/network_virtual.hpp"
#include "nfs/graphics/backend/eagl_state.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/core/frame/game_step.hpp"
#include <cstring>
#include <initializer_list>

namespace nfs::lifecycle::construction {
extern Dword particle_head, particle_tail;
}
namespace nfs::lifecycle::runtime {
Dword *&particle_owner = reinterpret_cast<Dword *&>(construction::particle_head);
Dword &particle_emitters = construction::particle_tail;
void *animation_buffer = nullptr;
volatile Dword animation_cursors[4] = {};
Dword input_frame_counts[2] = {};
Dword *console_objects[3] = {};
CRITICAL_SECTION crypt_lock{};
Dword *crypt_head = nullptr;
} // namespace nfs::lifecycle::runtime
namespace nfs::lifecycle::table_entries {
using namespace runtime;
namespace {
Dword *fields(System *object) {
    return reinterpret_cast<Dword *>(object);
}
std::uint8_t &byte(System *object, unsigned offset) {
    return reinterpret_cast<std::uint8_t *>(object)[offset];
}
void invoke(Dword *object, unsigned slot) {
    using Call = void(__thiscall *)(Dword *);
    reinterpret_cast<const Call *>(object[0])[slot](object);
}
} // namespace

void __fastcall initialize_particles(System *object, void *) {
    particle_owner = fields(object);
    particle_emitters = reinterpret_cast<Dword>(manager->slots[140 / 4]) + 24;
}

void __fastcall shutdown_particles(System *, void *) {
    particle_emitters = 0;
    particle_owner = nullptr;
}

void __fastcall initialize_skids(System *object, void *) {
    auto *state = fields(object);
    state[3] = 0;
    state[1] = 0;
    state[2] = 0;
    state[4] = 0;
    state[23] = 0;
    state[24] = 0;
}

void __fastcall initialize_events(System *object, void *) {
    auto *state = fields(object);
    state[1] = 0;
    kernel::memory_zero(state + 2, 3600);
    kernel::memory_zero(state + 3608 / 4, 500);
    kernel::memory_zero(state + 4108 / 4, 20);
}

void __fastcall initialize_speech(System *object, void *) {
    fields(object)[5] = 0;
}

void __fastcall initialize_skeleton(System *, void *) {
    Dword base = reinterpret_cast<Dword>(animation_buffer);
    animation_cursors[2] = base;
    animation_cursors[3] = base + 8192;
    animation_cursors[0] = base;
    animation_cursors[1] = base + 8192;
}
// Retain the actual conditional stores, not an invented reset.
void __fastcall enter_skeleton_frontend(System *, void *) {
    if (!animation_cursors[2])
        animation_cursors[2] = 0;
    if (!animation_cursors[3])
        animation_cursors[3] = 0;
    if (!animation_cursors[0])
        animation_cursors[0] = 0;
    if (!animation_cursors[1])
        animation_cursors[1] = 0;
}

void __fastcall load_skeleton_level(System *object, void *) {
    Dword *state = fields(object);
    for (unsigned i = 0; i < 64; ++i)
        state[24592 / 4 + i] = 0;
    state[25008 / 4] = state[25012 / 4] = 0;
    *reinterpret_cast<std::uint16_t *>(state + 25016 / 4) = 0;
}
// The first dereference precedes the null check.
void __fastcall leave_camera_frontend(System *, void *) {
    invoke(static_cast<Dword *>(game::tick_services[0]), 4);
    if (game::tick_services[0])
        gui::destroy(static_cast<gui::Object *>(game::tick_services[0]));
    game::tick_services[0] = nullptr;
}

void __fastcall leave_car_frontend(System *object, void *) {
    gui::application_release(reinterpret_cast<void *>(fields(object)[5]));
    fields(object)[5] = 0;
}

void __fastcall load_network_level(System *object, void *) {
    fields(object)[3] = 1;
    fields(object)[20] = 0;
}

void __fastcall unload_network_level(System *object, void *) {
    auto *state = fields(object);
    reinterpret_cast<std::uint16_t *>(state + 2)[0] = 0;
    reinterpret_cast<std::uint16_t *>(state + 2)[1] = 0;
    state[3] = state[8] = state[7] = 0;
}
// Complete tail forwarding to the race delegate's slot 15.
void __fastcall load_race_level(System *object, void *) {
    if (fields(object)[1])
        invoke(reinterpret_cast<Dword *>(fields(object)[1]), 15);
}

void __fastcall reset_input_frame(System *object, void *) {
    input_frame_counts[0] = input_frame_counts[1] = 0;
    auto *state = fields(object);
    for (unsigned i = 0; i < 2; ++i)
        state[1 + i] = state[4 + i] = state[7 + i] = 0;
}

void __fastcall shutdown_track_manager(System *object, void *) {
    auto *state = fields(object);
    while (state[1]) {
        Dword index = --state[1];
        gui::application_release(reinterpret_cast<void *>(state[index + 3]));
        state[index + 3] = 0;
    }
}
 
void __fastcall track_update(System *, void *) {
}

void __fastcall shutdown_client(System *object, void *) {
    *reinterpret_cast<std::uint16_t *>(reinterpret_cast<std::uint8_t *>(object) + 376) = 0;
    fields(object)[17] = 0;
}

void __fastcall shutdown_console(System *, void *) {
    for (unsigned index : {0u, 2u, 1u})
        if (console_objects[index]) {
            auto *object = reinterpret_cast<gui::Object *>(console_objects[index]);
            void *released = object->methods[0](object, 0);
            gui::application_release(released);
            console_objects[index] = nullptr;
        }
}

void __fastcall shutdown_netcrypt(System *, void *) {
    EnterCriticalSection(&crypt_lock);
    for (Dword *node = crypt_head; node;) {
        Dword *current = node;
        node = reinterpret_cast<Dword *>(node[12]);
        gui::destroy(reinterpret_cast<gui::Object *>(current));
    }
    LeaveCriticalSection(&crypt_lock);
    DeleteCriticalSection(&crypt_lock);
}

void __fastcall shutdown_feedback(System *object, void *) {
    if (static_cast<int>(fields(object)[1]) >= 0) {
        input::disable_feedback(fields(object)[1]);
        fields(object)[1] = 4294967295u;
    }
}

void __fastcall shutdown_replay(System *object, void *) {
    if (fields(object)[1])
        kernel::memory_free(reinterpret_cast<void *>(fields(object)[1]));
    fields(object)[1] = 0;
}

void __fastcall load_replay_level(System *object, void *) {
    fields(object)[10] = fields(object)[11] = 0;
}

void __fastcall unload_start_line(System *object, void *) {
    if (fields(object)[1])
        gui::destroy(reinterpret_cast<gui::Object *>(fields(object)[1]));
    fields(object)[1] = 0;
}

} // namespace nfs::lifecycle::table_entries
