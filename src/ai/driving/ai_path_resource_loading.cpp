#include "../population/ai_population_runtime.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cstring>
#include <cmath>

#include "../population/ai_population_x87.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
namespace {
extern const Dword section_methods[1];
}

void *construct_path_section(void *section) {
    at<Dword>(section, 36) = at<Dword>(section, 40) = 0;
    at<const Dword *>(section, 0) = section_methods;
    return section;
}

void *__fastcall delete_path_section(void *section, void *, Byte flags) {
    at<const Dword *>(section, 0) = section_methods;
    if (void *nodes = at<void *>(section, 40))
        kernel::memory_free(nodes);
    if (flags & 1)
        gui::application_release(section);
    return section;
}
namespace {
const Dword section_methods[1] = {reinterpret_cast<Dword>(delete_path_section)};
}

void read_path_node(void *node, const Dword **cursor) {
    const Dword *source = *cursor;
    const Dword x = source[0];
    const float right = at<float>(source, 16);
    at<Dword>(node, 0) = x;
    at<float>(node, 16) = right;
    const Dword y = source[1];
    const float left = at<float>(source, 12);
    at<Dword>(node, 4) = y;
    at<float>(node, 12) = left;
    at<Dword>(node, 8) = source[2];
    at<Dword>(node, 20) = source[5];
    at<Dword>(node, 24) = source[6];
    if (left < 0)
        at<Dword>(node, 12) = 0;
    at<float>(node, 16) = right;
    *cursor = source + 7;
    if (0 < right)
        at<Dword>(node, 16) = 0;
}

void read_path_section(void *section, const Dword **cursor) {
    const Dword *source = *cursor;
    at<Dword>(section, 4) = source[0];
    at<Dword>(section, 8) = source[1];
    const Dword third = source[2], fourth = source[3];
    at<Dword>(section, 12) = third;
    at<Dword>(section, 16) = fourth;
    at<Dword>(section, 28) = source[6];
    at<Dword>(section, 32) = source[7];
    const int count = int(source[8]);
    at<int>(section, 36) = count;
    *cursor = source + 9;
    if (count) {
        void *nodes = kernel::memory_allocate("AI::Segments", int(Dword(count) << 5),
                                              nfs::lifecycle::level_pool);
        at<void *>(section, 40) = nodes;
        int index = 0;
        if (count > 0) {
            do {
                void *node = bytes(at<void *>(section, 40)) + Dword(index) * 32;
                if (node) {
                    for (unsigned field = 12; field < 32; field += 4)
                        at<Dword>(node, field) = 0;
                    at<Dword>(node, 0) = 0;
                    at<Dword>(node, 4) = 0;
                    at<Dword>(node, 8) = 0;
                }
                read_path_node(bytes(at<void *>(section, 40)) + Dword(index) * 32, cursor);
                ++index;
            } while (index < at<int>(section, 36));
        }
    }
}

void *first_path_node(void *section) {
    return at<int>(section, 36) > 0 ? at<void *>(section, 40) : nullptr;
}
// Forward the receiver and its argument to the resource loader.
long double path_edge_length(const float *node, const float *previous) {
    return runtime::point_distance(node, previous);
}

void build_path_arc_distances(void *section) {
    population_frame::build_path_arc_distances(section);
}

void *construct_path_vertex(void *vertex, const float *position) {
    at<Dword>(vertex, 0) = at<Dword>(position, 0);
    const Dword y = at<Dword>(position, 4);
    const Dword z = at<Dword>(position, 8);
    at<Dword>(vertex, 4) = y;
    at<std::uint16_t>(vertex, 44) = 0;
    at<Dword>(vertex, 8) = z;
    return vertex;
}
namespace {
} // namespace

Byte insert_path_section(void *group, void *section) {
    return population_frame::insert_path_section(group, section);
}
// Complete RET4 leaf.
void finish_path_group(void *, Dword) {
}
} // namespace nfs::game::runtime::ai::lifecycle::pending

namespace nfs::game::world::pending {

void load_ai_path_groups(void *manager) {
    using namespace runtime::ai::lifecycle::pending;
    auto *image = static_cast<Dword *>(files::file::try_load(runtime::level_file("aipaths.dat"),
                                                            nfs::lifecycle::level_pool | 256));
    if (!image)
        return;
    const Dword *cursor = image + 1;
    if (*image == 1) {
        void *group = offset(manager, 4);
        for (int pass = 0; pass < 2; ++pass) {
            const Dword points = *cursor;
            const int count = int(cursor[points * 3 + 1]);
            cursor += points * 3 + 2;
            for (int remaining = count; remaining > 0; --remaining) {
                void *memory =
                    nfs::lifecycle::allocate(48, "AI::Path", nfs::lifecycle::level_pool);
                void *section = memory ? construct_path_section(memory) : nullptr;
                read_path_section(section, &cursor);
                if (!insert_path_section(group, section) && section)
                    invoke(section, 0, Dword{1});
            }
            if (launch::field<Dword>(8627900))
                finish_path_group(group, 0);
            group = offset(group, 42080);
        }
    }
    kernel::memory_free(image);
}
} // namespace nfs::game::world::pending
