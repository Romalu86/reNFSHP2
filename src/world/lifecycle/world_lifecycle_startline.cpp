#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
namespace nfs::game::world {


void load_start_line(void *owner) {
    if (void *old = at<void *>(owner, 4)) {
        invoke<void *>(old, 0, 1u);
        at<void *>(owner, 4) = nullptr;
    }
    for (int index = 0; index < at<int>(subsystem(24), 80); ++index) {
        auto *marker = static_cast<Dword *>(
            runtime::level_marker(static_cast<Dword *>(subsystem(24)), 5, index));
        if (!marker)
            continue;
        float orientation[9];
        quaternion_matrix(reinterpret_cast<float *>(marker + 3), orientation);
        orientation[2] = -orientation[2];
        orientation[5] = -orientation[5];
        orientation[6] = -orientation[6];
        orientation[7] = -orientation[7];
        auto *object =
            static_cast<Dword *>(lifecycle::allocate(68, "StartLine", gui::application_pool));
        if (object) {
            object[1] = 0;
            object[2] = at<Dword>(owner, 4);
            object[5] = marker[2];
            object[4] = marker[1];
            object[3] = marker[0];
            std::memcpy(object + 6, orientation, 36);
            object[15] = object[16] = 0;
            object[0] = reinterpret_cast<Dword>(pending::start_line_methods);
            object[15] = 1342177279;
            object[16] = 2668564255u;
            pending::build_start_line_geometry(object);
        }
        at<void *>(owner, 4) = object;
    }
}

void prepare_start_line_object(void *object) {
    if (void *next = at<void *>(object, 8))
        prepare_start_line_object(next);
    pending::build_start_line_geometry(static_cast<Dword *>(object));
}

void prepare_start_line(void *owner) {
    if (void *object = at<void *>(owner, 4)) {
        if (void *next = at<void *>(object, 8))
            prepare_start_line_object(next);
        pending::build_start_line_geometry(static_cast<Dword *>(object));
    }
}
} // namespace nfs::game::world
