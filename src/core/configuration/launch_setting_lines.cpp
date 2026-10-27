#include "nfs/core/configuration/launch_options.hpp"
#include <cctype>
namespace nfs::launch::settings {
int remaining{};
char *cursor{}, *allocation{};


Dword next(char *name, char *value, int *section) {
    char *name_end = name, *value_end = value;
    bool in_name = true;
    *name = *value = 0;
    *section = 0;
    if (!remaining)
        return 0;
    while (remaining) {
        const signed char character = *cursor++;
        --remaining;
        if (character == -1) {
            remaining = 0;
            return 0;
        }
        if (character == '[' && !*name && !*value) {
            *section = 1;
            continue;
        }
        if (*section && character == ']') {
            while (remaining > 0) {
                const signed char trailing = *cursor++;
                --remaining;
                if (trailing == -1) {
                    remaining = 0;
                    break;
                }
                if (trailing == '\r')
                    break;
            }
            break;
        }
        if (character == '\r') {
            if (*name || *value)
                break;
            continue;
        }
        if (character == '\n' || (in_name && (character == '\t' || character == ' ')))
            continue;
        if (character == ';') {
            while (remaining > 0) {
                const signed char trailing = *cursor++;
                --remaining;
                if (trailing == -1) {
                    remaining = 0;
                    break;
                }
                if (trailing == '\r')
                    break;
            }
            if (*name || *value)
                break;
            continue;
        }
        if (character == '=' && in_name) {
            in_name = false;
            continue;
        }
        if (in_name)
            *name_end++ = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        else
            *value_end++ = static_cast<char>(character);
    }
    name_end[0] = name_end[1] = value_end[0] = value_end[1] = 0;
    return 1;
}
} // namespace nfs::launch::settings
