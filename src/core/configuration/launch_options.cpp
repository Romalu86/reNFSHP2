#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
#include <cstdio>
namespace nfs::launch {
unsigned char network_requested{};

namespace {
void apply_dependencies(Options *owner) {
    if (owner->flags[32])
        owner->flags[7] = 1;
    if (owner->flags[34]) {
        owner->flags[26] = owner->flags[22] = owner->flags[33] = 1;
        std::strcpy(owner->values[33], field<char[24]>(8628772));
    }
}
} // namespace

Dword parse(Options *owner, int count, const char *const *arguments) {
    Dword recognized = 1;
    for (int argument = 0; argument < count; ++argument) {
        int entry = 0;
        while (specifications[entry].flag != -1) {
            if (!_stricmp(arguments[argument] + 1, specifications[entry].name)) {
                owner->flags[specifications[entry].flag] = 1;
                if (argument + 1 < count && arguments[argument + 1][0] != '-')
                    std::sprintf(owner->values[entry], arguments[argument + 1]);
                break;
            }
            if (!_stricmp(arguments[argument], "-host")) {
                owner->flags[22] = owner->flags[24] = 1;
                network_requested = 1;
                break;
            }
            if (!_stricmp(arguments[argument], "-connect")) {
                if (argument + 1 < count && arguments[argument + 1][0] != '-') {
                    owner->flags[24] = 1;
                    std::sprintf(owner->values[24], arguments[argument + 1]);
                    network_requested = 1;
                }
                break;
            }
            if (!_stricmp(arguments[argument], "-gameport")) {
                if (argument + 1 < count && arguments[argument + 1][0] != '-') {
                    owner->flags[31] = 1;
                    std::sprintf(owner->values[31], arguments[argument + 1]);
                }
                break;
            }
            const bool player_name = !_stricmp(arguments[argument], "-name");
            const bool password = !_stricmp(arguments[argument], "-password");
            if (player_name || password) {
                char value[500]{};
                if (collect_quoted_value(count, arguments, &argument, value)) {
                    const int destination = player_name ? 25 : 29;
                    owner->flags[destination] = 1;
                    std::strcpy(owner->values[destination], value);
                }
                break;
            }
            ++entry;
        }
        if (specifications[entry].flag == -1)
            recognized = 0;
    }
    apply_dependencies(owner);
    if (owner->flags[22])
        owner->flags[23] = 1;
    return recognized;
}

Dword parse_file(Options *owner) {
    Dword recognized = 1;
    if (settings::open("args.ini")) {
        char name[16], value[128];
        int section;
        settings::next(name, value, &section);
        while (settings::next(name, value, &section) && !section) {
            int entry = 0;
            for (; specifications[entry].flag != -1; ++entry)
                if (!_stricmp(name, specifications[entry].name)) {
                    owner->flags[specifications[entry].flag] = 1;
                    if (value[0])
                        std::sprintf(owner->values[entry], value);
                    break;
                }
            if (specifications[entry].flag == -1)
                recognized = 0;
        }
        settings::close();
    }
    apply_dependencies(owner);
    return recognized;
}
} // namespace nfs::launch
