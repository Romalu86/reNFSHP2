#include "../flow/frontend_runtime.hpp"
#include "nfs/gui/resources/gui_format.hpp"
namespace nfs::application::pending {
void *create_widget_class(const char *, void *);
Dword load_widget_children(void *, Dword);
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
void *find_child(void *, const char *);

void *create_interface_screen(const gui::String *name, const gui::String *type) {
    if (!gui::active_engine())
        return nullptr;
    alignas(4) Byte configuration[56];
    {
        LocalFrontendString file("System\\GInterface.gui");
        gui::engine::pending::create_configuration(configuration, &file.value);
    }
    void *widget = nullptr;
    try {
        if (gui::string_data(name) && gui::string_data(type)) {
            widget = create_widget_class(gui::string_ascii(type), nullptr);
            if (widget) {
                LocalFrontendString qualified;
                {
                    LocalFrontendString separator("."), prefix;
                    gui::concatenate(&prefix.value, name, &separator.value);
                    gui::concatenate(&qualified.value, &prefix.value, type);
                }
                gui::string_copy(reinterpret_cast<gui::String *>(bytes(widget) + 72), name);
                gui::engine::pending::select_section(configuration,
                                                     gui::string_ascii(&qualified.value));
                invoke(widget, 4, static_cast<void *>(configuration));
                load_widget_children(widget, 1);
                gui::events::propagate(widget, 18, 0, 0, 0);
            }
        }
    } catch (...) {
        gui::engine::pending::destroy_configuration(configuration);
        throw;
    }
    gui::engine::pending::destroy_configuration(configuration);
    return widget;
}

void add_frontend_chyron(void *, void *screen) {
    if (screen && !find_child(screen, "<CHYRON>")) {
        LocalFrontendString type("ChyronCtl"), name("<CHYRON>"), file("System\\GInterface.gui");
        create_widget(&file.value, &name.value, &type.value, screen);
    }
}
} // namespace nfs::application::pending
