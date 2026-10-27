#include "frontend_runtime.hpp"
#include "nfs/audio/music/music_clients.hpp"
namespace nfs::application::pending {
namespace {
struct OwnedArgument {
    gui::String *value;
    ~OwnedArgument() {
        gui::string_destruct(value);
    }
};
struct ConfigurationOwner {
    alignas(4) Byte data[56];
    ConfigurationOwner() {
        LocalFrontendString file("lists.ini");
        gui::engine::pending::create_configuration(data, &file.value);
    }
    ~ConfigurationOwner() {
        gui::engine::pending::destroy_configuration(data);
    }
};
void set_player_names(Dword player, const gui::String *name) {
    set_preference_name(player_preferences(system(72), player), name);
    set_preference_network_name(player_preferences(system(72), player), name);
}
} // namespace
// The queue head stays owned by the screen callbacks.
int run_screen_queue(void *frontend, gui::String name, gui::String type) {
    OwnedArgument type_argument{&type}, name_argument{&name};
    {
        LocalFrontendString copied_type(&type), copied_name(&name);
        enqueue_screen(frontend, copied_name.release(), copied_type.release());
    }
    ConfigurationOwner configuration;
    int result = 7;
    bool done = screen_queue_empty(frontend);
    while (!done) {
        auto *queue = at<gui::List *>(frontend, 48);
        if (!queue)
            return 7;
        auto *command = static_cast<ScreenCommand *>(gui::events::peek_front(queue));
        if (!command)
            return 7;
        bool invalid;
        {
            LocalFrontendString copy(&command->name);
            invalid = !gui::string_data(&copy.value);
        }
        if (invalid)
            return 7;
        {
            LocalFrontendString copy(&command->type);
            invalid = !gui::string_data(&copy.value);
        }
        if (invalid)
            return 7;
        {
            LocalFrontendString copy(&command->name);
            invalid = !copy.value.length;
        }
        if (invalid)
            return 7;
        {
            LocalFrontendString copy(&command->type);
            invalid = !copy.value.length;
        }
        if (invalid)
            return 7;
        {
            LocalFrontendString copied_type(&command->type), copied_name(&command->name);
            at<void *>(frontend, 40) =
                create_interface_screen(&copied_name.value, &copied_type.value);
        }
        void *screen = at<void *>(frontend, 40);
        if (!screen)
            return 7;
        add_frontend_chyron(frontend, screen);
        if (profile_prompt_pending) {
            profile_prompt_pending = 0;
            LocalFrontendString placeholder;
            gui::string_reference(&placeholder.value, 2064);
            set_player_names(0, &placeholder.value);
            gui::string_reference(&placeholder.value, 454);
            set_player_names(1, &placeholder.value);
            if (!frontend_dependencies::prompt_load_profile(at<void *>(frontend, 40))) {
                LocalFrontendString player_name;
                gui::string_reference(&player_name.value, 2064);
                set_player_names(0, &player_name.value);
            }
        }
        if (!game::audio::state::library().playing)
            game::audio::select_music(reinterpret_cast<game::audio::MusicPlayer *>(system(80)),
                                      -1);
        screen = at<void *>(frontend, 40);
        result = gui::run_modal(screen, nullptr);
        destroy_screen(screen);
        at<void *>(frontend, 40) = nullptr;
        if (result != 16)
            done = screen_queue_empty(frontend);
    }
    return result;
}
} // namespace nfs::application::pending
