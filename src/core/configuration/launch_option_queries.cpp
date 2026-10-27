#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
namespace nfs::launch {
#include "launch_option_data.inc"

Options *construct(Options *owner) {
    kernel::memory_zero(owner, 308);
    kernel::memory_zero(owner->values, 1848);
    return owner;
}
// The bound has no negative-index check.
const char *option_name(int index) {
    return index > 76 ? nullptr : specifications[index].name;
}
// Only AL is defined by this query's callers.
unsigned char collect_quoted_value(int count, const char *const *arguments, int *index,
                                   char *output) {
    if (*index + 1 >= count || arguments[*index + 1][0] != '"')
        return 0;
    char joined[500]{};
    int quotes = 0;
    while (*index + 1 < count) {
        std::strcat(joined, arguments[++*index]);
        std::strcat(joined, " ");
        quotes = 0;
        for (unsigned i = 0; i < std::strlen(joined); ++i)
            if (joined[i] == '"')
                ++quotes;
        if (quotes >= 2)
            break;
    }
    if (quotes == 2) {
        char unquoted[500]{};
        std::strcpy(unquoted, joined + 1);
        for (unsigned i = 0; i < std::strlen(unquoted); ++i)
            if (unquoted[i] == '"') {
                unquoted[i] = 0;
                break;
            }
        if (std::strlen(unquoted)) {
            std::strcpy(output, unquoted);
            return 1;
        }
    }
    return 0;
}
} // namespace nfs::launch
