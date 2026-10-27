#include "gui_script.hpp"
namespace nfs::gui::widgets::script {

static void destroy_variable(Variable *variable) {
    string_destruct(&variable->value);
    string_destruct(&variable->name);
    variable->methods = gui::base_methods;
}

static Variable *__fastcall delete_variable(Variable *variable, void *, Dword flags) {
    destroy_variable(variable);
    if (flags & 1)
        release(variable);
    return variable;
}
const Destroy variable_methods[]{reinterpret_cast<Destroy>(delete_variable)};
namespace {
Variable *variable_at(Container *variables, Dword index) {
    return static_cast<Variable *>(events::container_value(variables, index));
}
Variable *new_variable() {
    auto *variable = static_cast<Variable *>(allocate_flags(sizeof(Variable), 0, "GScriptVar"));
    if (variable) {
        string_construct(&variable->name);
        string_construct(&variable->value);
        variable->methods = variable_methods;
    }
    return variable;
}
} // namespace

Script *find_script(const String *name) {
    if (!string_data(name))
        return nullptr;
    for (Dword index = 0; index < script_instances.count; ++index)
        if (auto *script = static_cast<Script *>(events::container_value(&script_instances, index)))
            if (string_equal(&script->uid, name))
                return script;
    return nullptr;
}

const String *find_global(const String *name) {
    if (name->length) {
        const Dword count = global_script_variables.count;
        for (Dword index = 0; index < count; ++index)
            if (auto *variable = variable_at(&global_script_variables, index))
                if (string_equal(&variable->name, name))
                    return &variable->value;
    }
    return empty();
}

const String *find_local(Script *script, const String *name) {
    if (name->length) {
        const Dword count = script->variables.count;
        for (Dword index = 0; index < count; ++index)
            if (auto *variable = variable_at(&script->variables, index))
                if (string_equal(&variable->name, name))
                    return &variable->value;
    }
    return empty();
}

const String *resolve(Script *script, const String *name) {
    const String *value = empty();
    if (name->length)
        for (Dword index = 0; index < script->variables.count; ++index)
            if (auto *variable = variable_at(&script->variables, index))
                if (string_equal(&variable->name, name)) {
                    value = &variable->value;
                    break;
                }
    if (!value->length) {
        value = find_global(name);
        if (!value->length)
            value = name;
    }
    return value;
}

void set_global(const String *name, const String *value) {
    if (!string_data(name))
        return;
    const Dword count = global_script_variables.count;
    for (Dword index = 0; index < count; ++index)
        if (auto *variable = variable_at(&global_script_variables, index))
            if (string_equal(&variable->name, name)) {
                if (value->length)
                    string_copy(&variable->value, value);
                else
                    container_remove_at(&global_script_variables, index);
                return;
            }
    if (string_data(value)) {
        auto *variable = new_variable();
        // Global setter copies through the allocation result without an extra guard.
        string_copy(&variable->name, name);
        string_copy(&variable->value, value);
        if (container_append(&global_script_variables, variable, dispose_object) && variable)
            destroy(reinterpret_cast<Object *>(variable));
    }
}

void set_local(Script *script, const String *name, const String *value) {
    if (!string_data(name))
        return;
    const Dword count = script->variables.count;
    for (Dword index = 0; index < count; ++index)
        if (auto *variable = variable_at(&script->variables, index))
            if (string_equal(&variable->name, name)) {
                if (value->length)
                    string_copy(&variable->value, value);
                else
                    container_remove_at(&script->variables, index);
                return;
            }
    if (value->length)
        if (auto *variable = new_variable()) {
            string_copy(&variable->name, name);
            string_copy(&variable->value, value);
            container_append(&script->variables, variable, dispose_object);
        }
}

const String *argument(Command *command, Dword index, Script *script) {
    auto *value = static_cast<String *>(events::container_value(&command->arguments, index));
    if (!value)
        return empty();
    return script ? resolve(script, value) : value;
}

Script *first_child_script(Widget *widget) {
    for (int index = 0; index < int(widget->children.count); ++index)
        if (Widget *child = child_at(widget, index, 0))
            if (game::runtime::invoke<Dword>(child, 2, class_key))
                return reinterpret_cast<Script *>(child);
    return nullptr;
}
} // namespace nfs::gui::widgets::script
