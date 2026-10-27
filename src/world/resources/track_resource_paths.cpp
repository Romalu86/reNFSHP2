#include "nfs/world/resources/runtime_resources.hpp"
#include <cstdio>
namespace nfs::game::runtime {
// This path has no levelNN component;  is the distinct variant helper.
const char *track_file(const char *name) {
    std::sprintf(pending::track_path, "%s%s\\%s",
                 reinterpret_cast<const char *>(system(4)) + 524,
                 reinterpret_cast<const char *>(system(40)) + 140, name);
    return pending::track_path;
}
} // namespace nfs::game::runtime
