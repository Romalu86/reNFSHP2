#include "nfs/world/resources/runtime_resources.hpp"
#include <cstdio>
namespace nfs::game::runtime {

// The writer is known; owning scratch capacity/startup allocation stays explicit.

const char *level_file(const char *name) {
    Dword *tracks = system(40);
    auto *descriptor = at<void *>(tracks, 12 + at<Dword>(tracks, 8) * 4);
    int variant = at<int>(descriptor, 20) % 3;
    int setting = static_cast<int>(network::packets::rule_value(system(72), 6));
    if (setting == 1 || static_cast<int>(network::packets::rule_value(system(72), 6)) == 3)
        variant += 3;
    char section[12];
    std::sprintf(section, "level%02d", variant);
    std::sprintf(pending::level_path, "%s%s\\%s\\%s",
                 reinterpret_cast<const char *>(system(4)) + 524,
                 reinterpret_cast<const char *>(tracks) + 140, section, name);
    return pending::level_path;
}
// No negative type/index checks were added.
void *level_marker(Dword *manager, int kind, int index) {
    if (at<int>(manager, 60 + kind * 4) <= index)
        return nullptr;
    return at<Byte *>(manager, 84 + kind * 4) + index * 36;
}
} // namespace nfs::game::runtime
