#include "controller_configuration.hpp"
namespace nfs::profiles::pending {

Byte configure_profile_input() {
    void *controller_list = controllers::get_bindings();
    controllers::get_definitions();
    controllers::load_default_maps(controllers::get_default_maps());
    void *default_maps = controllers::get_default_maps();
    void *preferences = bytes(system(72)) + 2488;
    Dword first_device = Dword(-1), last_device = Dword(-1);
    bool reset = !at<Byte>(preferences, 56);
    if (!reset) {
        struct DeviceUse {
            Dword device;
            Byte used;
            Byte padding[3];
        } used[3];
        for (auto &entry : used) {
            entry.device = Dword(-1);
            entry.used = 0;
        }
        int used_count = 0;
        for (unsigned action = 0; action < 26 && !reset; ++action) {
            const Byte type = at<Byte>(preferences, 56 + action * 4);
            if (type == 1 || type == 2 || type == 3)
                last_device = at<Dword>(preferences, 56 + action * 4) >> 28;
            if (last_device == Dword(-1))
                continue;
            if (first_device == Dword(-1))
                first_device = last_device;
            if (used_count < 3) {
                bool duplicate = false;
                for (int index = 0; index < used_count; ++index)
                    if (used[index].used && used[index].device == last_device) {
                        duplicate = true;
                        break;
                    }
                if (!duplicate) {
                    used[used_count].device = last_device;
                    used[used_count].used = 1;
                    ++used_count;
                    if (!controllers::matches_saved_controller(controller_list, last_device,
                                                               bytes(preferences) + 49364))
                        reset = true;
                }
            }
        }
    }
    if (!reset) {
        controllers::select_primary_controller(controller_list, int(first_device));
        return 0;
    }
    reset_controller_preferences(bytes(preferences) + 49364);
    reset_secondary_bindings(bytes(system(72)) + 52060);
    int selected_device = -1;
    void *map = nullptr;
    if (void *entry = controllers::controller_at(controllers::get_bindings(), 0)) {
        if (void *definition = at<void *>(entry, 4)) {
            selected_device = at<int>(entry, 0);
            map = controllers::find_default_map(
                default_maps, reinterpret_cast<const char *>(bytes(definition) + 8));
        }
    } else
        map = controllers::find_default_map(default_maps, "keyboard");
    capture_controller_preferences(controller_list, bytes(preferences) + 49364);
    if (map) {
        controllers::select_primary_controller(controller_list, selected_device);
        void *definition = at<void *>(map, 0);
        void *axes = invoke<void *>(definition, 6), *povs = invoke<void *>(definition, 8),
             *buttons = invoke<void *>(definition, 7);
        invoke(definition, 5);
        auto *entry = bytes(map) + 4;
        for (unsigned index = 0; index < 48; ++index, entry += 12) {
            const Dword action = at<Dword>(entry, 0), type = at<Dword>(entry, 4),
                       element = at<Dword>(entry, 8);
            controllers::action_name(int(action));
            void *binding = bytes(preferences) + 56 + action * 4;
            if (type == 2) {
                at<Byte>(binding, 0) = 4;
                at<Byte>(binding, 1) = Byte(element);
            } else if (type == 1) {
                const void *axis = bytes(axes) + 8 + element * 24;
                const Dword group = at<Dword>(axis, 4), first = at<Dword>(axis, 16),
                           second = at<Dword>(axis, 20);
                at<Byte>(binding, 0) = 1;
                at<Byte>(binding, 2) = Byte(first);
                at<Byte>(binding, 1) = Byte(second);
                at<Dword>(binding, 0) = (at<Dword>(binding, 0) & 16777215) ^ ((group & 15) << 24) ^
                                       (Dword(selected_device) << 28);
            } else if (type == 4) {
                const void *pov = bytes(povs) + 8 + element * 16;
                const Dword group = at<Dword>(pov, 4), direction = at<Dword>(pov, 12);
                at<Byte>(binding, 0) = 3;
                const Dword packed = Dword(controllers::pov_direction(int(direction)));
                at<Dword>(binding, 0) =
                    ((group & 15) << 24) ^
                    ((((packed & 65535) << 8) ^ (at<Dword>(binding, 0) & 4278190335u)) & 16777215) ^
                    (Dword(selected_device) << 28);
            } else if (type == 3) {
                const Dword group = at<Dword>(buttons, 12 + element * 16);
                at<Byte>(binding, 0) = 2;
                at<Dword>(binding, 0) = ((group & 1048575) << 8) ^ (at<Dword>(binding, 0) & 255) ^
                                       (Dword(selected_device) << 28);
            }
        }
    }
    return 1;
}
} // namespace nfs::profiles::pending
