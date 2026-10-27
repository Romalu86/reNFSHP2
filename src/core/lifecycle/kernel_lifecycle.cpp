#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/core/timing/timer.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/resources/localization/region.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/graphics/fonts/fonts.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "nfs/input/window/window_input.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::lifecycle {

void *allocate(int bytes, const char *name, Dword flags) {
    void *result = kernel::memory_allocate(name, bytes, flags);
    if (gui::allocation_break_address && result == gui::allocation_break_address)
        kernel::debug_break();
    return result;
}

void create_class(int bytes, const char *name, Dword *out, int aligned) {
    if (!bytes) {
        *out = ~Dword(0);
        return;
    }
    int id = kernel::pool_unused();
    if (id) {
        void *memory = kernel::memory_allocate(name, bytes, 1024);
        if (memory) {
            kernel::memory_fill(memory, 3203383023u, bytes);
            int result = kernel::pool_create(id, name, memory, bytes, aligned ? 16 : 4, 16, 4, true,
                                             true, true);
            if (result)
                *out = id;
        }
    }
}

void create_classes() {
    create_class(10485760, "CLASS_AUDIO", &audio_pool, 0);
    create_class(45088768, "CLASS_TRACK", &track_pool, 0);
    create_class(1662976, "CLASS_LEVEL", &level_pool, 0);
    create_class(region::get() == 6 ? 10485760 : 4194304, "CLASS_UI", &ui_pool, 0);
    create_class(7340032, "CLASS_CARS", &cars_pool, 0);
    create_class(region::get() == 6 ? 15933440 : 7544832, "CLASS_RENDER", &gui::application_pool,
                 1);
    create_class(1560576, "CLASS_CHARACTER", &character_pool, 0);
    create_class(409600, "CLASS_REPLAY", &replay_pool, 0);
    create_class(0, "CLASS_DEBUG", &debug_pool, 0);
    create_class(65536, "CLASS_INIFILE", &ini::load_pool, 0);
    create_class(0, "CLASS_SCREENSHOT", &screenshot_pool, 0);
    create_class(0, "CLASS_SAVELOAD", &save_pool, 0);
    kernel::memory_largest_free(0);
    current_pool = gui::application_pool;
}

void memory_bootstrap() {
    kernel::memory_initialize(region::get() == 6 ? 115052544 : 100372480);
    create_classes();
}
// The GUI result is ignored in the original.
Manager *initialize() {
    gui::application_host = {gui::host_unused, gui::create_lock, gui::enter, gui::leave,
                             gui::delete_lock};
    gui::initialize(&gui::application_host, 4194304, nullptr, "1.32.20-R");
    if (!manager) {
        auto *allocated = static_cast<Manager *>(allocate(148, "System Manager", level_pool));
        Manager *constructed = nullptr;
        if (allocated) {
            try {
                constructed = construct_manager(allocated);
            } catch (...) {
                // Run the cleanup handler before rethrowing.
                game::runtime::track::pending::release_named(allocated, "System Manager",
                                                                      level_pool);
                throw;
            }
        }
        manager = constructed;
        Manager *initial = manager;
        for (int i = 0; i < 37; ++i) {
            System *system = initial->slots[i];
            if (system)
                system->methods->initialize(system);
            if (startup_error)
                return manager;
        }
        if (!graphics::device) {
            Manager *initial_post = manager;
            for (int i = 0; i < 37; ++i) {
                System *system = initial_post->slots[i];
                if (system)
                    system->methods->post_initialize(system);
            }
            ini::flush_cache();
        }
        frame_clock::initialize();
    }
    return manager;
}
// Two reverse passes, then GUI cleanup.
void shutdown() {
    Manager *initial = manager;
    for (int i = 37; i;) {
        System *system = initial->slots[--i];
        if (system)
            system->methods->shutdown(system);
    }
    initial = manager;
    if (initial) {
        for (int i = 37; i;) {
            System *system = initial->slots[--i];
            if (system)
                system->methods->destroy(system, 1);
        }
        gui::application_release(initial);
    }
    manager = nullptr;
    gui::shutdown();
}
 
void platform_initialize() {
    kernel::timer_frequency = 120;
    kernel::thread_initialize();
    kernel::timer_initialize(kernel::timer_frequency);
    files::set_memory(kernel::memory_allocate, kernel::memory_free);
    files::initialize(0, 1000, 0);
    files::async::initialize(16, level_pool);
    int result = graphics::initialize();
    if (!startup_error) {
        startup_error = result;
        if (!result) {
            if (!window_input::initialize_hook()) {
                if (!startup_error)
                    startup_error = 7;
            } else {
                if (graphics::device) {
                    input::keyboard_initialize();
                    input::mouse_initialize();
                    input::devices_initialize();
                }
                files::archives::initialize();
                result = files::cache::application::initialize();
                if (!startup_error)
                    startup_error = result;
            }
        }
    }
}
// Kept separate from the testable component teardown.
void platform_shutdown() {
    windows_hook_error = 0;
    if (windows_hook) {
        UnhookWindowsHookEx(windows_hook);
        windows_hook_error = GetLastError();
    }
    windows_hook = nullptr;
    if (windows_hook_module)
        FreeLibrary(windows_hook_module);
    windows_hook_module = nullptr;
    files::cache::application::shutdown();
    files::archives::shutdown();
    fonts::shutdown();
    graphics::shutdown();
}
} // namespace nfs::lifecycle
