#pragma once
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::lifecycle {
using kernel::Dword;
struct System;
using SystemCall = void(__thiscall *)(System *);
struct SystemMethods {
    void *(__thiscall *destroy)(System *, unsigned);
    SystemCall initialize, shutdown, load_level, unload_level, reset_level, enter_frontend,
        leave_frontend, post_initialize;
};
struct System {
    const SystemMethods *methods;
};
struct Manager {
    System *slots[37];
};
static_assert(sizeof(Manager) == 148 && sizeof(SystemMethods) == 36);
extern Manager *manager;
extern int startup_error;
extern Dword &dedicated;
extern Dword track_pool, level_pool, cars_pool, audio_pool, debug_pool, character_pool, replay_pool,
    ui_pool, screenshot_pool, save_pool, current_pool;
extern HHOOK windows_hook;
extern HMODULE windows_hook_module;
extern Dword windows_hook_error;
extern Dword windows_hook_install_error;
void create_class(int, const char *, Dword *, int);
void create_classes();
void memory_bootstrap();
void *allocate(int, const char *, Dword);
Manager *initialize();
void shutdown();
void platform_initialize();
void platform_shutdown();
// The four entries consume their sole Manager receiver in ECX.
void __fastcall load_level(Manager *);
void __fastcall unload_level(Manager *);
void __fastcall enter_frontend(Manager *);
void __fastcall leave_frontend(Manager *);
extern std::uint8_t level_load_failed;
namespace phase_dependencies {
void report_memory_usage();
void finalize_vehicle_load();
void finish_level_load();
void set_track_phase(int);
} // namespace phase_dependencies
} // namespace nfs::lifecycle
