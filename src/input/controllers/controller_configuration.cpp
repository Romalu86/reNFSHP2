#include "controller_configuration.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include <cctype>
#include <cstring>
namespace nfs::controllers {

void *get_default_maps() {
    if (!profiles) {
        void *list = lifecycle::allocate(200, "Controllers::DefaultMapList", lifecycle::ui_pool);
        if (list) {
            at<Dword>(list, 192) = 0;
            at<Byte>(list, 196) = 0;
            for (unsigned index = 0; index < 48; ++index)
                at<void *>(list, index * 4) = nullptr;
        }
        profiles = list;
    }
    return profiles;
}
// The saved dereference after allocation failure is retained.
void *get_definitions() {
    if (!devices) {
        void *list = lifecycle::allocate(200, "Controllers::Definitions", lifecycle::ui_pool);
        if (list) {
            at<Byte>(list, 0) = 0;
            at<Dword>(list, 4) = 0;
            for (unsigned index = 0; index < 48; ++index)
                at<void *>(list, 8 + index * 4) = nullptr;
        }
        devices = list;
    }
    if (!at<Byte>(devices, 0))
        load_definitions(devices);
    return devices;
}
// Legacy CRT isupper mapping is an explicit locale boundary.
char *lower_controller_name(char *text) {
    for (char *cursor = text; *cursor; ++cursor)
        if (std::isupper(static_cast<unsigned char>(*cursor)))
            *cursor = char(*cursor + 32);
    return text;
}

int pov_direction(int direction) {
    switch (direction) {
    case 1:
        return 0;
    case 2:
        return 18000;
    case 3:
        return 27000;
    case 4:
        return 9000;
    }
    return -1;
}

const char *action_name(int action) {
    for (const auto &entry : input_actions) {
        if (!entry.name)
            break;
        if (entry.action == action)
            return entry.name;
    }
    return nullptr;
}

void *find_definition(void *owner, const char *name) {
    if (!name)
        return nullptr;
    for (Dword index = 0; index < at<Dword>(owner, 4); ++index)
        if (void *definition = at<void *>(owner, 8 + index * 4))
            if (!std::strcmp(name, reinterpret_cast<const char *>(bytes(definition) + 8)))
                return definition;
    return nullptr;
}
// Searches all48 allocated slots, independently of count.
void *find_default_map(void *owner, const char *name) {
    for (unsigned index = 0; index < 48; ++index)
        if (void *map = at<void *>(owner, index * 4))
            if (void *definition = at<void *>(map, 0))
                if (!std::strcmp(name, reinterpret_cast<const char *>(bytes(definition) + 8)))
                    return map;
    return nullptr;
}

Byte add_controller(void *owner, const char *name, Dword device) {
    const Dword count = at<Dword>(owner, 0);
    if (count < 3) {
        void *definitions = get_definitions();
        if (name)
            for (Dword index = 0; index < at<Dword>(definitions, 4); ++index)
                if (void *definition = at<void *>(definitions, 8 + index * 4))
                    if (!std::strcmp(name, reinterpret_cast<const char *>(bytes(definition) + 8))) {
                        at<Dword>(owner, 0) = count + 1;
                        at<void *>(owner, count * 8 + 8) = definition;
                        at<Dword>(owner, count * 8 + 4) = device;
                        return 1;
                    }
    }
    return 0;
}

void *controller_at(void *owner, Dword index) {
    return index < at<Dword>(owner, 0) ? bytes(owner) + index * 8 + 4 : nullptr;
}
// Only AL is defined on unsuccessful comparison paths.
Byte matches_saved_controller(void *owner, Dword device, const void *saved) {
    const int count = at<int>(saved, 204);
    for (int index = 0; index < count; ++index) {
        const void *entry = bytes(saved) + unsigned(index) * 68;
        if (at<Dword>(entry, 64) == device) {
            for (Dword current = 0; current < at<Dword>(owner, 0); ++current)
                if (void *definition = at<void *>(owner, current * 8 + 8))
                    if (!std::strcmp(reinterpret_cast<const char *>(bytes(definition) + 8),
                                     static_cast<const char *>(entry)))
                        return at<Dword>(owner, current * 8 + 4) == device;
            return 0;
        }
    }
    return 0;
}

void select_primary_controller(void *owner, int device) {
    at<int>(owner, 32) = device;
    at<int>(owner, 36) = -1;
    if (device != -1) {
        const int count = int(reinterpret_cast<Dword>(input::device_get(0, 1)));
        for (int index = 0; index < count; ++index)
            if (const void *about = input::device_get(index, 2))
                if ((at<Dword>(about, 4) & 3) == 3)
                    if (const void *state = input::device_get(index, 6))
                        if (at<Byte>(state, 0) == 4)
                            at<int>(owner, 36) = index;
    }
}

void clear_device_assignments(void *owner) {
    for (unsigned index = 0; index < 32; ++index) {
        at<Byte>(owner, 1012 + index * 8) = 0;
        at<Dword>(owner, 1016 + index * 8) = 0;
    }
}

void enable_device_assignment(void *owner, Dword device, Byte enabled) {
    if (device < 32)
        at<Byte>(owner, 1012 + device * 8) = enabled;
}
} // namespace nfs::controllers
