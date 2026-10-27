#include "../flow/frontend_runtime.hpp"
#include "nfs/gui/resources/gui_format.hpp"
namespace nfs::application::pending {
Byte initial_frontend_entry = 1, profile_prompt_pending = 1;
namespace {
struct OwnedArgument {
    gui::String *value;
    ~OwnedArgument() {
        gui::string_destruct(value);
    }
};
void construct_command(ScreenCommand *command, const gui::String *name, const gui::String *type) {
    LocalFrontendString copied_type(type), copied_name(name);
    command->methods = gui::base_methods;
    gui::string_construct(&command->name);
    try {
        gui::string_copy(&command->name, &copied_name.value);
        gui::string_construct(&command->type);
        try {
            gui::string_copy(&command->type, &copied_type.value);
        } catch (...) {
            gui::string_destruct(&command->type);
            throw;
        }
    } catch (...) {
        gui::string_destruct(&command->name);
        throw;
    }
    command->methods = screen_command_methods;
}
} // namespace

void destroy_screen_command(ScreenCommand *command) {
    gui::string_destruct(&command->type);
    gui::string_destruct(&command->name);
    command->methods = gui::base_methods;
}

void *__fastcall delete_screen_command(ScreenCommand *command, void *, unsigned flags) {
    destroy_screen_command(command);
    if (flags & 1)
        gui::release(command);
    return command;
}
const gui::Destroy screen_command_methods[1] = {
    reinterpret_cast<gui::Destroy>(delete_screen_command)};

gui::String *copy_screen_name(ScreenCommand *command, gui::String *output) {
    return gui::string_construct_copy(output, &command->name);
}

gui::String *copy_screen_type(ScreenCommand *command, gui::String *output) {
    return gui::string_construct_copy(output, &command->type);
}

void enqueue_screen(void *frontend, gui::String name, gui::String type) {
    OwnedArgument type_argument{&type}, name_argument{&name};
    auto *&queue = at<gui::List *>(frontend, 48);
    if (!queue) {
        auto *storage = static_cast<gui::List *>(gui::allocate(sizeof(gui::List)));
        if (storage) {
            try {
                gui::construct_default_list(storage);
            } catch (...) {
                gui::release(storage);
                throw;
            }
        }
        queue = storage;
        if (!queue)
            return;
    }
    auto *command = static_cast<ScreenCommand *>(gui::allocate(sizeof(ScreenCommand)));
    if (command) {
        try {
            construct_command(command, &name, &type);
        } catch (...) {
            gui::release(command);
            throw;
        }
    }
    gui::events::push_front(queue, command, gui::dispose_object);
}
// Preserve the captured queue through its disposal callbacks.
void clear_screen_queue(void *frontend) {
    if (auto *queue = at<gui::List *>(frontend, 48)) {
        gui::events::clear_list(queue);
        gui::destroy(reinterpret_cast<gui::Object *>(queue));
        at<gui::List *>(frontend, 48) = nullptr;
    }
}

bool screen_queue_empty(void *frontend) {
    auto *queue = at<gui::List *>(frontend, 48);
    return !queue || !gui::events::list_count(queue);
}
} // namespace nfs::application::pending
