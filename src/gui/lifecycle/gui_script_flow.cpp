#include "gui_script.hpp"
#include "../resources/gui_file_dialog.hpp"
#include "../../frontend/flow/frontend_toggle_widget.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::gui::widgets::script::commands {
using application::pending::LocalFrontendString;
using engine::pending::string_integer;

Dword __cdecl set(Script *script, Command *command, int) {
    LocalFrontendString name(argument(command, 0, nullptr));
    LocalFrontendString value(argument(command, 1, script));
    if (string_data(&name.value) && string_data(&value.value) &&
        !string_equal(&name.value, &value.value))
        set_local(script, &name.value, &value.value);
    return 1;
}

Dword __cdecl set_global_value(Script *script, Command *command, int) {
    LocalFrontendString name(argument(command, 0, nullptr));
    LocalFrontendString value(argument(command, 1, script));
    if (string_data(&name.value) && string_data(&value.value) &&
        !string_equal(&name.value, &value.value))
        set_global(&name.value, &value.value);
    return 1;
}

Dword __cdecl unset_global(Script *, Command *command, int) {
    LocalFrontendString name(argument(command, 0, nullptr));
    if (string_data(&name.value))
        set_global(&name.value, empty());
    return 1;
}

Dword __cdecl unset(Script *script, Command *command, int) {
    LocalFrontendString name(argument(command, 0, nullptr));
    if (string_data(&name.value))
        set_local(script, &name.value, empty());
    return 1;
}

Dword __cdecl target_sibling(Script *script, Command *command, int) {
    const String *name = argument(command, 0, script);
    if (!string_data(name))
        return 1;
    if (Widget *sibling = family::named_sibling(&script->base, name)) {
        script->target = sibling->id;
        return 1;
    }
    script->state = 0;
    return 0;
}

Dword __cdecl target_parent(Script *script, Command *, int) {
    if (script->base.parent) {
        script->target = script->base.parent->id;
        return 1;
    }
    script->state = 0;
    return 0;
}

Dword __cdecl trigger_script(Script *script, Command *command, int) {
    const String *name = argument(command, 0, script);
    if (string_data(name))
        if (auto *other = find_script(name))
            start(other, 0);
    return 1;
}

Dword __cdecl end_script(Script *script, Command *command, int) {
    const String *name = argument(command, 0, script);
    if (string_data(name))
        if (auto *other = find_script(name); other && other != script)
            other->state = 0;
    return 1;
}

Dword __cdecl go_to(Script *script, Command *command, int) {
    const String *position = argument(command, 0, script);
    if (string_data(position)) {
        script->position = Dword(string_integer(position));
        return 0;
    }
    return 1;
}

Dword __cdecl end(Script *script, Command *, int) {
    script->state = 0;
    return 0;
}

Dword __cdecl restart(Script *script, Command *, int) {
    start(script, 0);
    return 0;
}

Dword __cdecl wait(Script *script, Command *command, int ticks) {
    Widget *widget = target(script);
    const String *duration = argument(command, 0, script);
    if (string_data(duration) && widget) {
        float seconds = 0;
        if (std::sscanf(string_ascii(duration), "%f", &seconds)) {
            command->elapsed = int(Dword(command->elapsed) + Dword(ticks));
            if (command->elapsed < engine::seconds_to_ticks(seconds))
                return 0;
            command->elapsed = 0;
        }
    }
    return 1;
}

Dword __cdecl conditional(Script *script, Command *command, int) {
    LocalFrontendString left(argument(command, 0, script));
    LocalFrontendString operation(argument(command, 1, nullptr));
    LocalFrontendString right(argument(command, 2, script));
    if (!string_data(&left.value) || !string_data(&operation.value) || !string_data(&right.value))
        return 1;
    trim_leading_spaces(&operation.value);
    trim_trailing_spaces(&operation.value);
    trim_leading_spaces(&left.value);
    trim_trailing_spaces(&right.value);
    int position = -1;
    const int count = int(script->commands.count);
    for (int index = 0; index < count; ++index)
        if (events::container_value(&script->commands, Dword(index)) == command)
            position = index;
    if (position < 0)
        return 1;

    auto is_operator = [&](const char *name) {
        return application::frontend::toggle_string_equals(&operation.value, name);
    };
    if (is_operator("="))
        string_equal(&left.value, &right.value);
    bool skip = false;
    if (is_operator("!="))
        skip = string_equal(&left.value, &right.value);
    else if (is_operator(">="))
        skip = !string_greater_signed(&left.value, &right.value) &&
               !string_equal(&left.value, &right.value);
    else if (is_operator(">"))
        skip = !string_greater_signed(&left.value, &right.value);
    else if (is_operator("<="))
        skip = !string_less_signed(&left.value, &right.value) &&
               !string_equal(&left.value, &right.value);
    else if (is_operator("<"))
        skip = !string_less_signed(&left.value, &right.value);
    if (skip)
        script->position = Dword(position) + 2;
    return 1;
}
} // namespace nfs::gui::widgets::script::commands
