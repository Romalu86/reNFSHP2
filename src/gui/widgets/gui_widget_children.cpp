#include "gui_widgets.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::pending {
Dword split_widget_name(const char *, gui::String *, gui::String *);
void *create_widget_class(const char *, void *);

Dword load_widget_children(void *storage, Dword recursive) {
    auto *widget = static_cast<gui::widgets::Widget *>(storage);
    const gui::String *filename =
        widget->resource ? &widget->resource->string
                         : &lifecycle::construction::unresolved::shared_string;
    if (!gui::string_data(filename))
        return 8;
    if (!gui::string_data(&widget->name))
        return 7;
    gui::String type, name;
    gui::string_construct(&type);
    gui::string_construct(&name);
    alignas(4) Byte configuration[56];
    try {
        gui::engine::pending::create_configuration(configuration, filename);
        try {
            const int count = game::runtime::invoke<int>(configuration, 1);
            for (int index = 0; index < count; ++index) {
                const auto *section =
                    game::runtime::invoke<const gui::String *>(configuration, 2, index);
                if (section->length) {
                    const Dword section_wide = section->wide;
                    const char *qualified = gui::string_ascii(section);
                    gui::engine::pending::select_section(configuration, qualified);
                    const char *selected_name = section_wide
                                                    ? nullptr
                                                    : static_cast<const char *>(
                                                          gui::string_data(section));
                    if (split_widget_name(selected_name, &name, &type))
                        if (auto *child = static_cast<gui::widgets::Widget *>(
                                create_widget_class(gui::string_ascii(&type), widget))) {
                            gui::string_copy(&child->name, &name);
                            game::runtime::invoke(child, 4, static_cast<void *>(configuration));
                            if (recursive) {
                                const gui::String *resource =
                                    child->resource ? &child->resource->string
                                                    : &lifecycle::construction::unresolved::
                                                          shared_string;
                                if (resource->length)
                                    child->options |= 8;
                            }
                        }
                }
            }
        } catch (...) {
            gui::engine::pending::destroy_configuration(configuration);
            throw;
        }
        gui::engine::pending::destroy_configuration(configuration);
    } catch (...) {
        gui::string_destruct(&name);
        gui::string_destruct(&type);
        throw;
    }
    gui::string_destruct(&name);
    gui::string_destruct(&type);
    if (recursive) {
        const int count = int(widget->children.count);
        for (int index = 0; index < count; ++index)
            if (auto *child = static_cast<gui::widgets::Widget *>(
                    gui::events::container_value(&widget->children, Dword(index))))
                if (child->options & 8) {
                    child->options &= ~Dword(8);
                    load_widget_children(child, 1);
                }
    }
    return 0;
}
} // namespace nfs::application::pending
