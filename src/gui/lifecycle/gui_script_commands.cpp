#include "gui_script.hpp"
#include "gui_configuration_cache.hpp"
#include "../resources/gui_file_dialog.hpp"
#include "../../frontend/screens/frontend_dialogs.hpp"
#include "../../frontend/flow/frontend_simulation.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdio>
#include <cstdlib>
#include <climits>
namespace nfs::gui::widgets::script {
Widget *target(Script *script) {
    return static_cast<Widget *>(events::find_widget(context_state, script->target));
}
namespace commands {
using application::pending::LocalFrontendString;
using engine::pending::string_integer;
using engine::pending::string_number;
using game::runtime::invoke;
namespace {
Dword fixed_value(long double value) {
    const auto format = kernel::numeric::current_format();
    return Dword((kernel::numeric::Extended::from_double(double(value), format) * 65536.0f)
                    .truncate_to_integer());
}
} // namespace

Dword __cdecl set_hook(Script *script, Command *command, int) {
    Widget *widget = target(script);
    const String *event = argument(command, 0, script), *callback = argument(command, 1, script);
    if (string_data(event) && string_data(callback) && widget) {
        Dword event_hash, callback_hash;
        engine::pending::hash_name(&event_hash, string_ascii(event));
        engine::pending::hash_name(&callback_hash, string_ascii(callback));
        if (event_hash && callback_hash)
            for (auto *record = callbacks; record; record = record->next)
                if (record->event_hash == event_hash && record->callback_hash == callback_hash &&
                    invoke<Dword>(widget, 2, record->type)) {
                    application::pending::set_widget_callback(widget, record->event,
                                                              record->callback);
                    break;
                }
    }
    return 1;
}

Dword __cdecl set_text(Script *script, Command *command, int) {
    Widget *widget = target(script);
    const String *text = argument(command, 0, script);
    if (string_data(text) && widget)
        invoke(widget, 7, text);
    return 1;
}

Dword __cdecl set_user_data(Script *script, Command *command, int) {
    Widget *widget = target(script);
    const String *value = argument(command, 0, script);
    if (string_data(value) && widget)
        widget->value_84 = Dword(std::atol(string_ascii(value)));
    return 1;
}

Dword __cdecl set_colour(Script *script, Command *command, int) {
    Widget *widget = target(script);
    const String *value = argument(command, 0, script);
    if (string_data(value) && widget) {
        Dword colour = 0;
        if (std::sscanf(string_ascii(value), "%x", &colour))
            widget->colour = colour;
    }
    return 1;
}

Dword __cdecl set_position(Script *script, Command *command, int) {
    Widget *widget = target(script);
    const String *left = argument(command, 0, script), *top = argument(command, 1, script);
    if (string_data(left) && string_data(top) && widget) {
        const int x = string_integer(left), y = string_integer(top);
        widget->bounds.left = x;
        widget->bounds.top = y;
    }
    return 1;
}

Dword __cdecl set_size(Script *script, Command *command, int) {
    Widget *widget = target(script);
    const String *width = argument(command, 0, script), *height = argument(command, 1, script);
    if (string_data(width) && string_data(height) && widget) {
        const int x = string_integer(width), y = string_integer(height);
        if (x >= 0 && y >= 0) {
            widget->bounds.width = x;
            widget->bounds.height = y;
        }
    }
    return 1;
}

Dword __cdecl set_left(Script *script, Command *command, int) {
    Widget *widget = target(script);
    const String *value = argument(command, 0, script);
    if (string_data(value) && widget)
        widget->bounds.left = string_integer(value);
    return 1;
}

Dword __cdecl set_top(Script *script, Command *command, int) {
    Widget *widget = target(script);
    const String *value = argument(command, 0, script);
    if (string_data(value) && widget)
        widget->bounds.top = string_integer(value);
    return 1;
}

Dword __cdecl set_width(Script *script, Command *command, int) {
    Widget *widget = target(script);
    const String *value = argument(command, 0, script);
    if (string_data(value) && widget) {
        const int width = string_integer(value);
        if (width >= 0 && widget->bounds.height >= 0)
            widget->bounds.width = width;
    }
    return 1;
}

Dword __cdecl set_height(Script *script, Command *command, int) {
    Widget *widget = target(script);
    const String *value = argument(command, 0, script);
    if (string_data(value) && widget) {
        const int previous_width = widget->bounds.width;
        const int height = string_integer(value);
        if (previous_width >= 0 && height >= 0)
            widget->bounds.height = height;
    }
    return 1;
}

Dword __cdecl hide(Script *script, Command *, int) {
    if (Widget *widget = target(script))
        application::pending::set_widget_visible(widget, 0);
    return 1;
}

Dword __cdecl show(Script *script, Command *, int) {
    if (Widget *widget = target(script))
        application::pending::set_widget_visible(widget, 1);
    return 1;
}

Dword __cdecl enable(Script *script, Command *, int) {
    if (Widget *widget = target(script))
        set_enabled(widget, 1);
    return 1;
}

Dword __cdecl disable(Script *script, Command *, int) {
    if (Widget *widget = target(script))
        set_enabled(widget, 0);
    return 1;
}

Dword __cdecl focus(Script *script, Command *, int) {
    if (Widget *widget = target(script))
        activate(widget);
    return 1;
}
// Dispatch the frame-rate setting to the target widget.
Dword __cdecl set_fps(Script *script, Command *command, int) {
    LocalFrontendString value(argument(command, 0, script));
    if (Widget *widget = target(script))
        if (string_data(&value.value))
            events::dispatch(widget, 53, fixed_value(string_number(&value.value)), 0);
    return 1;
}

Dword __cdecl set_max_characters(Script *script, Command *command, int) {
    LocalFrontendString value(argument(command, 0, script));
    if (Widget *widget = target(script))
        if (string_data(&value.value)) {
            const int limit = string_integer(&value.value);
            if (limit >= 0)
                events::dispatch(widget, 58, Dword(limit), 0);
        }
    return 1;
}

Dword __cdecl set_transform(Script *script, Command *command, int) {
    LocalFrontendString value(argument(command, 0, script));
    if (Widget *widget = target(script))
        if (string_data(&value.value))
            events::dispatch(widget, 51, fixed_value(string_number(&value.value)), 2);
    return 1;
}

Dword __cdecl set_instance_name(Script *script, Command *command, int) {
    LocalFrontendString name(argument(command, 0, script));
    if (Widget *widget = target(script))
        if (!family::named_sibling(widget, &name.value))
            string_copy(&widget->name, &name.value);
    return 1;
}

Dword __cdecl create(Script *script, Command *command, int) {
    LocalFrontendString resource(argument(command, 0, script));
    LocalFrontendString name(argument(command, 1, script));
    LocalFrontendString type(argument(command, 2, script));
    if (Widget *widget = target(script))
        if (string_data(&resource.value) && string_data(&name.value) && string_data(&type.value))
            if (!application::pending::find_child(widget, string_ascii(&name.value)))
                application::pending::create_widget(&resource.value, &name.value, &type.value,
                                                    widget);
    return 1;
}
} // namespace commands
} // namespace nfs::gui::widgets::script
