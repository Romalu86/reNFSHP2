#include "controller_interfaces.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
namespace nfs::controllers {

int load_default_maps(void *owner) {
    if (at<Byte>(owner, 196))
        return 0;
    const void *paths = system(4);
    at<Byte>(owner, 196) = 1;
    int loaded = 0;
    char filename[260];
    std::sprintf(filename, "%s%s", reinterpret_cast<const char *>(bytes(paths) + 3904),
                 "defaults.ini");
    if (!files::file::exists(filename))
        return 0;
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        for (int index = 0; index < 48; ++index) {
            char section[140], name[256];
            std::sprintf(section, "joystick %d", index);
            if (!ini::get_string(&reader, section, "name", name, 255))
                break;
            lower_controller_name(name);
            auto *map = static_cast<Dword *>(
                lifecycle::allocate(580, "Controllers::DefaultMap", lifecycle::ui_pool));
            if (map) {
                map[0] = 0;
                for (unsigned binding = 0; binding < 48; ++binding) {
                    map[1 + binding * 3] = Dword(-1);
                    map[2 + binding * 3] = 0;
                    map[3 + binding * 3] = Dword(-1);
                }
                void *keyboard = keyboard_controller();
                void *definition = keyboard;
                if (std::strcmp(name, reinterpret_cast<const char *>(bytes(keyboard) + 8)))
                    definition = find_definition(get_definitions(), name);
                map[0] = reinterpret_cast<Dword>(definition);
            }
            const Dword count = at<Dword>(owner, 192);
            at<void *>(owner, count * 4) = map;
            at<Dword>(owner, 192) = count + 1;
            // The bound includes words after the null-name sentinel; default_map_fields contains those
            // pointer and word values.
            auto *binding = map;
            for (const InputAction &field : default_map_fields) {
                char text[256];
                if (!ini::get_string(&reader, section, field.name, text, 255))
                    break;
                if (char *token = std::strtok(text, " ")) {
                    Dword type = 0;
                    if (!std::strcmp(token, "axis"))
                        type = 1;
                    else if (!std::strcmp(token, "key"))
                        type = 2;
                    else if (!std::strcmp(token, "button"))
                        type = 3;
                    else if (!std::strcmp(token, "pov"))
                        type = 4;
                    binding[2] = type;
                    binding[1] = Dword(field.action);
                    if ((token = std::strtok(nullptr, " ")) != nullptr) {
                        if (type == 2)
                            binding[3] = Dword(key_from_name(token));
                        else if (type == 1 || type == 3 || type == 4)
                            binding[3] = Dword(std::atoi(token));
                    }
                }
                binding += 3;
            }
            ++loaded;
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
    return loaded;
}
} // namespace nfs::controllers
