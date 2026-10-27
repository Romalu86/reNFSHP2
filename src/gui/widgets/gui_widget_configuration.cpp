#include "gui_widget_settings.hpp"
#include "gui_widget_hooks.hpp"
#include "../lifecycle/gui_configuration_cache.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::engine::pending {
ini::Document *acquire_configuration_document(const String *);
void release_configuration_document(const String *);
} // namespace nfs::gui::engine::pending
namespace nfs::gui::widgets {
using game::runtime::invoke;
using namespace engine::pending;
#include "gui_widget_flags.inc"
namespace {
const String *empty_text() {
    return &lifecycle::construction::unresolved::shared_string;
}
// C++ automatic cleanup expresses the saved string unwind states.
struct LocalString : String {
    LocalString() {
        string_construct(this);
    }
    ~LocalString() {
        string_destruct(this);
    }
};
void save_prefixed_hook(void *configuration, const char *prefix, const char *event,
                        const char *value, const char *type) {
    LocalString start;
    string_assign(&start, prefix);
    String key;
    append_ascii_value(&key, &start, event);
    try {
        write_ascii(configuration, string_ascii(&key), value, type, 0, nullptr);
    } catch (...) {
        string_destruct(&key);
        throw;
    }
    string_destruct(&key);
}
} // namespace
namespace pending {

Dword __fastcall load_configuration(Widget *widget, void *, void *configuration) {
    LocalString font, palette, children;
    read_string(configuration, "CHILDREN", &children, empty_text());
    const String *previous = widget->resource ? &widget->resource->string : empty_text();
    if (!string_equal(previous, &children)) {
        if (widget->options & 16) {
            release_configuration_document(previous);
            widget->options &= ~Dword(16);
        }
        string_release(widget->resource);
        widget->resource = string_acquire(string_ascii(&children));
        if (container_mode) {
            acquire_configuration_document(widget->resource ? &widget->resource->string
                                                            : empty_text());
            widget->options |= 16;
        }
    }
    read_flags(configuration, "GUI.mLayout", &widget->layout_flags, layout_flags, 0);
    read_integer(configuration, "GUI.mZOrder", &widget->z_order, 0);
    read_integer(configuration, "GUI.mTabOrder", &widget->tab_index, 0);
    read_flags(configuration, "GUI.mStyle", &widget->flags, widget_flags, 0);
    read_rectangle(configuration, "GUI.mBounds", &widget->bounds, &engine::default_bounds);
    read_string(configuration, "GUI.mFont", &font, empty_text());
    read_string(configuration, "GUI.mPalette", &palette, empty_text());
    Dword colour = Dword(-1);
    read_colour(configuration, "GUI.mColor", &widget->colour, &colour);
    read_reference_string(configuration, "GUI.mText", &widget->text, empty_text());
    read_reference_string(configuration, "GUI.mToolTip", &widget->tooltip, empty_text());
    if (auto *engine = active_engine()) {
        if (font.length)
            widget->font_style = invoke<Dword>(engine, 7, &font);
        if (palette.length)
            widget->palette = invoke<Dword>(engine, 19, &palette);
    }
    if (widget->parent) {
        container_remove(&widget->parent->draw_order, widget);
        container_sorted_insert(&widget->parent->draw_order, compare_order,
                                reinterpret_cast<void *>(Dword(widget->z_order)), widget, nullptr);
        container_remove(&widget->parent->tab_order, widget);
        container_sorted_insert(&widget->parent->tab_order, compare_tab,
                                reinterpret_cast<void *>(Dword(widget->tab_index)), widget, nullptr);
    }
    if (widget->handlers)
        container_clear(widget->handlers);
    const int count = invoke<int>(configuration, 3);
    for (int index = 0; index < count; ++index) {
        const String *key = invoke<const String *>(configuration, 4, index);
        if (character_at(key, 0) == '*' && !key->wide) {
            if (const char *name = string_ascii(key)) {
                const String *value = invoke<const String *>(configuration, 6, index);
                Dword event_hash, callback_hash;
                hash_name(&event_hash, name + 1);
                hash_name(&callback_hash, string_ascii(value));
                if (event_hash && callback_hash)
                    for (CallbackRecord *record = callbacks; record; record = record->next)
                        if (event_hash == record->event_hash &&
                            callback_hash == record->callback_hash &&
                            invoke<Dword>(widget, 2, record->type)) {
                            const Dword event = record->event;
                            const Callback callback = record->callback;
                            if (!widget->handlers)
                                widget->handlers = create_hook_container();
                            if (widget->handlers)
                                set_hook_callback(widget->handlers, event, callback);
                            break;
                        }
            }
        }
        if (character_at(key, 0) == '#' && !key->wide) {
            if (const char *name = string_ascii(key)) {
                const String *value = invoke<const String *>(configuration, 6, index);
                if (character_at(value, 0) && name[1]) {
                    Dword command_hash;
                    hash_name(&command_hash, string_ascii(value));
                    const Dword event = find_registered_event(name + 1);
                    if (event && find_registered_name(command_hash)) {
                        // Preserve the ASCII pointer obtained before hook allocation.
                        const char *command = string_ascii(value);
                        if (!widget->handlers)
                            widget->handlers = create_hook_container();
                        if (widget->handlers) {
                            hash_name(&command_hash, command);
                            set_hook_command(widget->handlers, event, command_hash);
                        }
                    }
                }
            }
        }
    }
    return 0;
}

Dword __fastcall save_configuration(Widget *widget, void *, void *configuration) {
    LocalString font, palette;
    if (auto *engine = active_engine()) {
        String name;
        invoke<String *>(engine, 8, &name, widget->font_style);
        try {
            string_copy(&font, &name);
        } catch (...) {
            string_destruct(&name);
            throw;
        }
        string_destruct(&name);
        invoke<String *>(engine, 20, &name, widget->palette);
        try {
            string_copy(&palette, &name);
        } catch (...) {
            string_destruct(&name);
            throw;
        }
        string_destruct(&name);
    }
    const String *children = widget->resource ? &widget->resource->string : empty_text();
    write_ascii(configuration, "CHILDREN", string_ascii(children), "FILE", 0, nullptr);
    write_flags(configuration, "GUI.mLayout", widget->layout_flags, layout_flags, 0);
    write_integer(configuration, "GUI.mZOrder", widget->z_order, 0);
    write_integer(configuration, "GUI.mTabOrder", widget->tab_index, 0);
    write_flags(configuration, "GUI.mStyle", widget->flags, widget_flags, 0);
    write_rectangle(configuration, "GUI.mBounds", &widget->bounds, &engine::default_bounds);
    write_ascii(configuration, "GUI.mFont", string_ascii(&font), "FONT", 0, nullptr);
    write_ascii(configuration, "GUI.mPalette", string_ascii(&palette), "PALETTE", 0, nullptr);
    Dword colour = Dword(-1);
    write_colour(configuration, "GUI.mColor", &widget->colour, &colour);
    write_reference_string(configuration, "GUI.mText", &widget->text, empty_text());
    write_reference_string(configuration, "GUI.mToolTip", &widget->tooltip, empty_text());
    Container *handlers = widget->handlers;
    if (handlers) {
        for (CallbackRecord *record = callbacks; record; record = record->next)
            if (record->callback && hook_callback(handlers, record->event) == record->callback)
                save_prefixed_hook(configuration, "*", record->event_name, record->callback_name,
                                   "HOOK");
        for (EventRecord *record = named_events; record; record = record->next) {
            const Dword command = hook_command(handlers, record->event);
            const char *name = find_registered_name(command);
            if (command && name)
                save_prefixed_hook(configuration, "#", record->name, name, "CMD");
        }
    }
    return 0;
}

Dword __fastcall load_general(GeneralWidget *widget, void *, void *configuration) {
    read_flags(configuration, "GWidget.mWidgetStyle", &widget->style, general_style_flags, 0);
    read_flags(configuration, "GWidget.mTabKeys", &widget->state, tab_key_flags, 31);
    return load_configuration(&widget->base, nullptr, configuration);
}

Dword __fastcall save_general(GeneralWidget *widget, void *, void *configuration) {
    write_flags(configuration, "GWidget.mWidgetStyle", widget->style, general_style_flags, 0);
    write_flags(configuration, "GWidget.mTabKeys", widget->state, tab_key_flags, 31);
    return save_configuration(&widget->base, nullptr, configuration);
}
} // namespace pending
} // namespace nfs::gui::widgets
