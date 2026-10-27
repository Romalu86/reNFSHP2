#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../race/rules/race_rules_x87.hpp"
#include "nfs/resources/files/filesystem.hpp"
#include "nfs/core/frame/game_step.hpp"
#include <cstdio>
#include <cmath>
namespace nfs::game::runtime::track {

int next_section(Dword *owner, int player) {
    const int current = at<int>(owner, 48 + player * 4);
    return current == at<int>(owner, 4) - 1 ? 0 : current + 1;
}

int previous_section(Dword *owner, int player) {
    int current = at<int>(owner, 48 + player * 4);
    if (!current)
        current = at<int>(owner, 4);
    return current - 1;
}

int previous_pair(Dword *owner, int player) {
    const int current = at<int>(owner, 48 + player * 4);
    if (current == 1)
        return at<int>(owner, 4) - 1;
    if (!current)
        return at<int>(owner, 4) - 2;
    return current - 2;
}

void *boundary(Dword *owner, int which, int player) {
    int section;
    if (!which)
        section = next_section(owner, player);
    else if (which == 1)
        section = at<int>(owner, 48 + player * 4);
    else if (which == 2)
        section = previous_section(owner, player);
    else
        return nullptr;
    return bytes(owner) + (player + section * 2) * 168 + 300;
}

void *spatial_cell(Dword *grid, const float *position) {
    const float *scale = at<const float *>(grid, 4);
    if (scale[0] == 0 || scale[1] == 0)
        return nullptr;
    const float *origin = at<const float *>(grid, 0);
    auto E = [](float value) { return static_cast<long double>(value); };
    const int x = hardware::current_integer_f32(float((E(position[0]) - origin[0]) / E(scale[0])));
    const int z = hardware::current_integer_f32(float((E(position[2]) - origin[1]) / E(scale[1])));
    const int *size = at<const int *>(grid, 8);
    if (x >= size[0] || z >= size[1] || x < 0 || z < 0)
        return nullptr;
    return bytes(at<void *>(grid, 16)) + (Dword(size[0]) * Dword(z) + Dword(x)) * 8u;
}

void *polygon_vertices(Dword *grid, const std::uint16_t *index, float **a, float **b, float **c,
                      float **d) {
    auto *vertex = bytes(at<void *>(at<void *>(grid, 12), 4)) + *index * 64;
    *a = reinterpret_cast<float *>(vertex);
    *b = reinterpret_cast<float *>(vertex + 12);
    *c = reinterpret_cast<float *>(vertex + 24);
    *d = reinterpret_cast<float *>(vertex + 36);
    return vertex;
}
// Only AL is a defined result at every caller.
Byte contains(Dword *grid, const float *position) {
    void *entry = spatial_cell(grid, position);
    if (!entry)
        return 0;
    const unsigned count = at<std::uint16_t>(entry, 0);
    auto *indices = at<const std::uint16_t *>(entry, 4);
    for (unsigned i = 0; i < count; ++i) {
        float *a, *b, *c, *d;
        polygon_vertices(grid, indices + i, &a, &b, &c, &d);
        const auto side = [position](const float *first, const float *second) {
            auto E = [](float value) { return static_cast<long double>(value); };
            const auto a = (E(first[0]) - second[0]) * (E(position[2]) - second[2]);
            const auto b = (E(position[0]) - second[0]) * (E(first[2]) - second[2]);
            return a <= b;
        };
        if (side(a, b) && side(b, c) && side(c, d) && side(d, a))
            return 1;
    }
    return 0;
}
} // namespace nfs::game::runtime::track
