#include "nfs/core/application/application.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstring>
namespace nfs::application::pending {
using game::runtime::at;

Dword capture_replay_length(void *replay) {
    const Dword record_count = at<Dword>(replay, 44);
    at<Dword>(replay, 48) = record_count;
    return record_count & 4294967040u;
}
void *create_widget_class(const char *, void *);

Dword split_widget_name(const char *qualified, gui::String *name, gui::String *type) {
    char copy[260];
    std::strcpy(copy, qualified);
    char *separator = std::strstr(copy, ".");
    if (!separator)
        return 0;
    gui::string_assign(type, separator + 1);
    *separator = 0;
    gui::string_assign(name, copy);
    return 1;
}

void *load_widget(const char *filename, const char *qualified, void *parent) {
    if (!filename || !qualified)
        return nullptr;
    gui::String type, name, resource;
    gui::string_construct(&type);
    gui::string_construct(&name);
    gui::string_construct(&resource);
    alignas(4) Byte configuration[56];
    try {
        gui::string_assign(&resource, filename);
        gui::engine::pending::create_configuration(configuration, &resource);
    } catch (...) {
        gui::string_destruct(&resource);
        gui::string_destruct(&name);
        gui::string_destruct(&type);
        throw;
    }
    gui::string_destruct(&resource);
    void *widget = nullptr;
    try {
        gui::engine::pending::select_section(configuration, qualified);
        if (split_widget_name(qualified, &name, &type)) {
            widget = create_widget_class(gui::string_ascii(&type), parent);
            if (widget) {
                gui::string_copy(
                    reinterpret_cast<gui::String *>(game::runtime::bytes(widget) + 72), &name);
                game::runtime::invoke(widget, 4, static_cast<void *>(configuration));
            }
        }
    } catch (...) {
        gui::engine::pending::destroy_configuration(configuration);
        gui::string_destruct(&name);
        gui::string_destruct(&type);
        throw;
    }
    gui::engine::pending::destroy_configuration(configuration);
    gui::string_destruct(&name);
    gui::string_destruct(&type);
    return widget;
}
} // namespace nfs::application::pending
