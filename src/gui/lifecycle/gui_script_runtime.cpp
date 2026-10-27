#include "gui_script.hpp"
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::network::session {
Dword pump_connection_dialog(void *);
}
namespace nfs::gui::widgets::script {

void advance(Script *script, int ticks) {
    if (script->state != 1 || script->busy)
        return;
    script->busy = 1;
    script->elapsed = int(Dword(script->elapsed) + Dword(ticks));
    while (script->position < script->commands.count) {
        auto *command =
            static_cast<Command *>(events::container_value(&script->commands, script->position));
        if (command) {
            if (!command->function || !command->function->call) {
                script->state = 0;
                break;
            }
            if (!command->function->call(script, command, ticks))
                break;
        }
        if (script->state != 1)
            break;
        ++script->position;
    }
    if (script->position >= script->commands.count)
        script->state = 2;
    script->busy = 0;
}

void __fastcall advance_frame(Script *script, void *, int ticks) {
    advance(script, ticks);
}

void modal_driver(void *owner) {
    auto *script = static_cast<Script *>(owner);
    while (script->state == 1)
        network::session::pump_connection_dialog(script);
    family::finish_modal(&script->base, 1, script->base.id);
}

void start(Script *script, Dword modal) {
    script->position = 0;
    script->state = 1;
    script->elapsed = 0;
    script->base.timer = 0;
    if (!contains_modal(&script->base) && (script->base.options & 2) && modal)
        run_modal(script, modal_driver);
    else
        advance(script, 0);
}

void pause(Script *script, Dword paused) {
    if (!paused) {
        if (script->state == 3)
            script->state = 1;
    } else if (script->state == 1) {
        script->state = 3;
    }
}

Dword __fastcall event(Script *script, void *, Dword event, Dword first, Dword second) {
    switch (event) {
    case 81:
        start(script, first);
        return 1;
    case 82:
        script->state = 0;
        return 1;
    case 83:
        pause(script, first);
        return 1;
    case 84:
        script->position = first;
        return 1;
    case 85:
        return script->position;
    case 86:
        script->target = first;
        return 1;
    case 87:
        return script->target;
    case 88:
        if (first) {
            application::pending::LocalFrontendString value(reinterpret_cast<const char *>(second));
            application::pending::LocalFrontendString name(reinterpret_cast<const char *>(first));
            set_local(script, &name.value, &value.value);
            return 1;
        }
        return 0;
    case 89:
        if (first) {
            application::pending::LocalFrontendString name(reinterpret_cast<const char *>(first));
            return reinterpret_cast<Dword>(string_ascii(find_local(script, &name.value)));
        }
        return 0;
    }
    if ((event == 19 && (script->style & 1)) ||
        (event == 24 && (script->style & 4) && second == script->base.id))
        start(script, script->style & 8);
    return pending::handle_event(&script->base, nullptr, event, first, second);
}
} // namespace nfs::gui::widgets::script
