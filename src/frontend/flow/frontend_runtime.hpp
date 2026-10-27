#pragma once
#include "nfs/core/application/application.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/gui/events/gui_events.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::application::pending {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
struct LocalFrontendString {
    gui::String value;
    bool owns_value = true;
    LocalFrontendString() {
        gui::string_construct(&value);
    }
    explicit LocalFrontendString(const char *text) {
        gui::string_construct_ascii(&value, text);
    }
    explicit LocalFrontendString(const gui::String *text) {
        gui::string_construct_copy(&value, text);
    }
    ~LocalFrontendString() {
        if (owns_value)
            gui::string_destruct(&value);
    }
    gui::String release() noexcept {
        owns_value = false;
        return value;
    }
    LocalFrontendString(const LocalFrontendString &) = delete;
    LocalFrontendString &operator=(const LocalFrontendString &) = delete;
};
struct ScreenCommand {
    const gui::Destroy *methods;
    gui::String name, type;
};
static_assert(sizeof(ScreenCommand) == 44 && offsetof(ScreenCommand, type) == 24);
extern const gui::Destroy screen_command_methods[1];
extern Byte initial_frontend_entry, profile_prompt_pending;
void enqueue_screen(void *, gui::String, gui::String);
int run_screen_queue(void *, gui::String, gui::String);
void clear_screen_queue(void *);
bool screen_queue_empty(void *);
gui::String *copy_screen_name(ScreenCommand *, gui::String *);
gui::String *copy_screen_type(ScreenCommand *, gui::String *);
void *player_preferences(void *, Dword);
gui::String *copy_preference_name(void *, gui::String *);
void set_preference_name(void *, const gui::String *);
void set_preference_network_name(void *, const gui::String *);
void *create_interface_screen(const gui::String *, const gui::String *);
void add_frontend_chyron(void *, void *);
namespace frontend_dependencies {
Byte prompt_load_profile(void *);
}
} // namespace nfs::application::pending
