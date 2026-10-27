#include "gui_script.hpp"
#include "gui_configuration_cache.hpp"
#include "gui_images.hpp"
#include "../widgets/gui_widget_edit.hpp"
#include <cstring>
namespace nfs::gui::widgets::script {
FunctionRecord *functions{};

void trim_leading_spaces(String *text) {
    while (text->length > 0 && engine::pending::character_at(text, 0) == ' ')
        erase_string_character(text, 0);
}

void trim_trailing_spaces(String *text) {
    while (text->length > 0 && engine::pending::character_at(text, text->length - 1) == ' ')
        erase_string_character(text, text->length - 1);
}

void trim_leading(String *text, const char *characters) {
    if (characters)
        while (text->length) {
            const Dword character = engine::pending::character_at(text, 0);
            if (!std::strchr(characters, int(character)))
                return;
            erase_string_character(text, 0);
        }
}

void trim_trailing(String *text, const char *characters) {
    if (characters)
        while (text->length) {
            const int last = int(Dword(text->length) - 1);
            const Dword character = engine::pending::character_at(text, last);
            if (!std::strchr(characters, int(character)))
                return;
            erase_string_character(text, last);
        }
}

void link_function(FunctionRecord *record) {
    record->next = functions;
    functions = record;
}

void register_function(FunctionRecord *record, const char *name, Handler handler) {
    record->next = nullptr;
    record->call = handler;
    engine::pending::hash_name(&record->hash, name);
    link_function(record);
}

void unregister_function(FunctionRecord *record) {
    FunctionRecord *previous = nullptr;
    for (auto *current = functions; current; current = current->next) {
        if (current == record) {
            if (previous)
                previous->next = record->next;
            else
                functions = record->next;
            return;
        }
        previous = current;
    }
}

Command *construct_command(Command *command, const String *text) {
    container_construct(&command->arguments);
    command->methods = command_methods;
    command->elapsed = 0;
    command->function = nullptr;
    try {
        if (!text->length)
            return command;
        application::pending::LocalFrontendString name, arguments;
        const int opening = string_find_character(text, '(');
        const int closing = string_find_character(text, ')');
        string_substring(text, 0, opening, &name.value);
        string_substring(text, opening + 1, closing - opening - 1, &arguments.value);
        if (name.value.length) {
            trim_leading(&name.value, "( \t\r\n");
            trim_trailing(&name.value, " \t\r\n)");
            if (!name.value.wide && string_data(&name.value)) {
                Dword hash;
                engine::pending::hash_name(&hash, string_ascii(&name.value));
                if (hash)
                    for (auto *record = functions; record && record->call && record->hash;
                         record = record->next)
                        if (record->hash == hash) {
                            command->function = record;
                            break;
                        }
            }
            if (command->function)
                while (arguments.value.length) {
                    int comma = string_find_character(&arguments.value, ',');
                    if (comma < 0)
                        comma = arguments.value.length;
                    application::pending::LocalFrontendString argument;
                    string_substring(&arguments.value, 0, comma, &argument.value);
                    if (argument.value.length) {
                        trim_leading(&argument.value, "( \t\r\n");
                        trim_trailing(&argument.value, " \t\r\n)");
                        if (argument.value.length) {
                            auto *copy =
                                static_cast<String *>(allocate_flags(sizeof(String), 0, "GUIStr"));
                            if (copy) {
                                try {
                                    string_construct_copy(copy, &argument.value);
                                } catch (...) {
                                    release(copy);
                                    throw;
                                }
                            }
                            if (container_append(&command->arguments, copy, dispose_object) && copy)
                                destroy(reinterpret_cast<Object *>(copy));
                        }
                    }
                    string_substring(&arguments.value, comma + 1, arguments.value.length,
                                     &arguments.value);
                }
        }
        return command;
    } catch (...) {
        container_destruct(&command->arguments);
        command->methods = gui::base_methods;
        throw;
    }
}

void destroy_command(Command *command) {
    command->methods = command_methods;
    container_destruct(&command->arguments);
    command->methods = gui::base_methods;
}

static Command *__fastcall delete_command(Command *command, void *, Dword flags) {
    destroy_command(command);
    if (flags & 1)
        release(command);
    return command;
}
const Destroy command_methods[]{reinterpret_cast<Destroy>(delete_command)};

void parse(Script *script, const String *text) {
    container_clear(&script->commands);
    if (!text->length)
        return;
    auto *copy =
        static_cast<char *>(kernel::memory_allocate(nullptr, text->length + 1, pool | 256));
    if (!copy)
        return;
    std::strcpy(copy, string_ascii(text));
    for (char *part = std::strtok(copy, ";"); part; part = std::strtok(nullptr, ";")) {
        application::pending::LocalFrontendString line(part);
        trim_leading(&line.value, " \t\r\n");
        trim_trailing(&line.value, " \t\r\n");
        if (line.value.length && engine::images::find_separator(&line.value, "//")) {
            auto *command =
                static_cast<Command *>(allocate_flags(sizeof(Command), 0, "GScriptCommand"));
            if (command) {
                try {
                    construct_command(command, &line.value);
                } catch (...) {
                    release(command);
                    throw;
                }
            }
            if (container_append(&script->commands, command, dispose_object) && command)
                destroy(reinterpret_cast<Object *>(command));
        }
    }
    kernel::memory_free(copy);
}

Dword execute_command(Command *command, Script *script, int ticks) {
    if (script && command->function && command->function->call)
        return command->function->call(script, command, ticks);
    script->state = 0;
    return 0;
}
} // namespace nfs::gui::widgets::script
