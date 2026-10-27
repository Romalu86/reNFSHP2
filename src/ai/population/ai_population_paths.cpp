#include "ai_population_runtime.hpp"
#include <cstring>
#include "ai_population_x87.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

void *construct_population_path(void *path) {
    for (unsigned offset = 4; offset <= 36; offset += 4)
        at<Dword>(path, offset) = 0;
    reset_path_reference(bytes(path) + 40);
    return path;
}
// Complete RET destructor.
void destroy_population_path(void *) {
}

void bind_population_location_path(void *location) {
    alignas(4) Byte reference[48], path[88];
    reset_path_reference(reference);
    float position[] = {at<float>(location, 16), at<float>(location, 20),
                        -at<float>(location, 24)};
    construct_population_path(path);
    try {
        population_frame::bind_location_path(location, reference, position, path);
    } catch (...) {
        destroy_population_path(path);
        throw;
    }
    destroy_population_path(path);
}

void *section_position(void *section, int node) {
    return bytes(at<void *>(section, 40)) + Dword(node) * 32;
}

void anchor_paths(Dword *manager) {
    population_frame::anchor_paths(manager);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
