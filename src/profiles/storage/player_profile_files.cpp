#include "player_profile.hpp"
#include <cstring>
namespace nfs::profiles {
void initialize_profile_settings(SavedProfile *profile, Byte valid) {
    at<Dword>(profile, 50088) = 0;
    at<Dword>(profile, 50136) = 1;
    for (unsigned offset = 50104; offset <= 50132; offset += 4)
        at<Dword>(profile, offset) = 100;
    at<Dword>(profile, 50140) = 0;
    for (unsigned offset = 50144; offset <= 50196; offset += 4)
        at<Dword>(profile, offset) = 1;
    at<Dword>(profile, 50200) = at<Dword>(profile, 50204) = 0;
    for (unsigned offset = 50208; offset <= 50224; offset += 4)
        at<Dword>(profile, offset) = 100;
    for (unsigned offset = 50228; offset <= 50256; offset += 4)
        at<Dword>(profile, offset) = 0;
    at<Dword>(profile, 50260) = 224;
    at<Byte>(profile, 50268) = valid;
}

SavedProfile *construct_profile(SavedProfile *profile) {
    construct_preferences(profile);
    initialize_profile_settings(profile, 1);
    reset_preferences(profile);
    return profile;
}

void destroy_profile(SavedProfile *profile) {
    destroy_preferences(profile);
}

void *delete_profile(SavedProfile *profile, unsigned flags) {
    destroy_profile(profile);
    if (flags & 1)
        gui::application_release(profile);
    return profile;
}

SavedProfile *load_profile(SavedProfile *profile, const char *filename) {
    construct_preferences(profile);
    initialize_profile_settings(profile, 0);
    reset_preferences(profile);
    void *file = files::file::try_load(filename, lifecycle::ui_pool);
    if (file) {
        if (kernel::memory_size(file) == file_size) {
            std::memcpy(profile, file, file_size);
            kernel::memory_free(file);
            bool valid = false;
            const int version = at<int>(profile, 50260);
            if (version >= 222 && version <= 224) {
                const Dword stored = at<Dword>(profile, 50264);
                at<Dword>(profile, 50264) = 0;
                valid = profile_checksum(profile) == stored;
                at<Dword>(profile, 50264) = stored;
            }
            at<Byte>(profile, 50268) = Byte(valid);
        } else
            gui::application_release(file);
    }
    return profile;
}

Byte apply_profile(SavedProfile *profile, int player) {
    const int version = at<int>(profile, 50260);
    if (version < 222 || version > 224)
        return 0;
    const Dword stored = at<Dword>(profile, 50264);
    at<Dword>(profile, 50264) = 0;
    const bool valid = profile_checksum(profile) == stored;
    at<Dword>(profile, 50264) = stored;
    if (!valid)
        return 0;
    if (player < 0 || player >= 2)
        player = 0;
    void *rules = system(72);
    void *caps = graphics::startup::display_caps;
    copy_profile_dwords(bytes(rules) + 2488 + unsigned(player) * preferences_size, profile,
                        preferences_size / sizeof(Dword));
    copy_profile_dwords(bytes(rules) + 2312, profile->data + preferences_size, 172 / sizeof(Dword));
    const int maximum_detail = 4 - (at<Byte>(caps, 760) & 1);
    if (at<int>(rules, 2384) > maximum_detail)
        at<int>(rules, 2384) = maximum_detail;
    const int maximum_shadow = int(((at<Dword>(caps, 584) & 131072) | 262144) >> 17);
    if (at<int>(rules, 2380) > maximum_shadow)
        at<int>(rules, 2380) = maximum_shadow;
    const int maximum_stencil =
        graphics::startup::stencil_enabled(&graphics::startup::settings, 0) ? 4 : 3;
    if (at<int>(rules, 2388) > maximum_stencil)
        at<int>(rules, 2388) = maximum_stencil;
    return 1;
}

void capture_profile(SavedProfile *profile, int player) {
    if (player < 0 || player >= 2)
        player = 0;
    void *rules = system(72);
    if (void *preferences = bytes(rules) + 2488 + unsigned(player) * preferences_size)
        copy_preferences(profile, preferences);
    rules = system(72);
    copy_profile_dwords(profile->data + preferences_size, bytes(rules) + 2312, 172 / sizeof(Dword));
}

Byte save_profile(SavedProfile *profile, const char *filename) {
    at<Byte>(profile, 50268) = 1;
    at<Dword>(profile, 50264) = 0;
    at<Dword>(profile, 50264) = profile_checksum(profile);
    return Byte(files::file::try_save(filename, profile, file_size));
}
} // namespace nfs::profiles
