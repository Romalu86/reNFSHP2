#include "../driving/ai_car_spawn_runtime.hpp"
#include "../../race/session/race_runtime.hpp"
#include "nfs/network/session/network_rules.hpp"
namespace nfs::game::runtime::ai::cars {

gui::String *profile_display_name(void *preferences, gui::String *result) {
    return gui::string_construct_ascii(result,
                                       reinterpret_cast<const char *>(bytes(preferences) + 26));
}
namespace {
void assign_profile_name(gui::String *destination, void *preferences) {
    gui::String name;
    profile_display_name(preferences, &name);
    try {
        gui::string_copy(destination, &name);
    } catch (...) {
        gui::string_destruct(&name);
        throw;
    }
    gui::string_destruct(&name);
}
} // namespace
// The model-name argument is not read by this body.
void register_spawned_participant(void *race, Dword index, Dword kind, const char *name, const char *,
                                  Dword flags, Dword checkpoint_time) {
    auto *record = application::pending::participant(race, index);
    record->kind = int(kind);
    Dword *settings = system(72);
    void *descriptor = index < 24 ? bytes(settings) + 248 + index * 84 : nullptr;
    bool use_profile;
    if (!network::session_active())
        use_profile = kind == 1 || kind == 2;
    else
        use_profile = at<Dword>(network::rules::car_settings(settings, index), 0) != 0;
    if (use_profile && at<Dword>(descriptor, 4) == 0) {
        Dword player;
        if (at<Dword>(settings, 204) == index) {
            player = 0;
            assign_profile_name(&record->name, bytes(settings) + 2488);
        } else {
            player = 1;
            if (at<Dword>(settings, 208) == index)
                assign_profile_name(&record->name, bytes(settings) + 52576);
        }
        if (network::session_active()) {
            gui::String display;
            profile_display_name(bytes(settings) + 2488 + player * 50088, &display);
            try {
                gui::string_make_narrow(&display);
                std::strncpy(reinterpret_cast<char *>(bytes(descriptor) + 24),
                             gui::string_ascii(&display), 26);
                at<Byte>(descriptor, 49) = 0;
            } catch (...) {
                gui::string_destruct(&display);
                throw;
            }
            gui::string_destruct(&display);
        }
    } else {
        std::strncpy(reinterpret_cast<char *>(bytes(descriptor) + 24), name, 26);
        at<Byte>(descriptor, 49) = 0;
        gui::String display;
        gui::string_construct_ascii(&display, name);
        try {
            gui::string_copy(&record->name, &display);
        } catch (...) {
            gui::string_destruct(&display);
            throw;
        }
        gui::string_destruct(&display);
    }
    record->identifier = int(index);
    record->reserved_358 = flags;
    record->checkpoint_time = int(checkpoint_time);
}
} // namespace nfs::game::runtime::ai::cars
