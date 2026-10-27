#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::gui::input {
extern const char *const text_names[2629];
}
namespace nfs::game::world::pending {
// The callers use the output word, not residual EAX.
void resolve_text_token(const char *name, Dword *out) {
    int index;
    if (std::sscanf(name, "kEnum%d", &index) == 1 && index >= 0 && index < 2629) {
        if (out)
            *out = Dword(index);
        return;
    }
    if (std::strstr(name, "kText") == name || std::strstr(name, "kTxt") == name ||
        std::strstr(name, "kMenu") == name) {
        for (Dword token = 0; token < 2629; ++token)
            if (!std::strcmp(name, gui::input::text_names[token])) {
                if (out)
                    *out = token;
                return;
            }
    }
    if (out)
        *out = Dword(-1);
}
} // namespace nfs::game::world::pending
