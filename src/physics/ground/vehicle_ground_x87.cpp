#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../motion/world_motion_x87.hpp"
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <utility>
namespace nfs::game::runtime::physics {
namespace {
bool edge_contains(const float *position, const float *first, const float *second) {
    auto E = [](float value) { return static_cast<long double>(value); };
    const auto a = (E(first[0]) - second[0]) * (E(position[2]) - second[2]);
    const auto b = (E(position[0]) - second[0]) * (E(first[2]) - second[2]);
    return a <= b;
}
} // namespace

Byte sample_ground(const float *position, void *contact) {
    const float *cached = at<const float *>(contact, 24);
    Byte surface = 0;
    if (cached) {
        const float *corners = reinterpret_cast<const float *>(bytes(contact) + 28);
        if (edge_contains(position, corners, corners + 3) &&
            edge_contains(position, corners + 3, corners + 6) &&
            edge_contains(position, corners + 6, corners + 9) &&
            edge_contains(position, corners + 9, corners) && cached[0] == corners[0])
            return 1;
        surface = at<Byte>(cached, 59) & 15;
    }
    if (split_screen()) {
        const std::uint16_t previous = at<std::uint16_t>(contact, 80);
        const int alternate = previous == 0;
        for (const auto query : {std::pair<int, int>{1, previous},
                                 {1, alternate},
                                 {0, previous},
                                 {2, previous},
                                 {0, alternate},
                                 {2, alternate}}) {
            auto *compartment =
                static_cast<Dword *>(track::boundary(system(36), query.first, query.second));
            if (compartment &&
                contacts_pending::search_ground_cell(track::spatial_cell(compartment, position),
                                                     compartment, position, contact, surface)) {
                at<std::uint16_t>(contact, 80) = static_cast<std::uint16_t>(query.second);
                return 1;
            }
        }
        at<std::uint16_t>(contact, 80) = previous;
    } else {
        for (int kind : {1, 0, 2}) {
            auto *compartment = static_cast<Dword *>(track::boundary(system(36), kind, 0));
            if (compartment &&
                contacts_pending::search_ground_cell(track::spatial_cell(compartment, position),
                                                     compartment, position, contact, surface)) {
                at<std::uint16_t>(contact, 80) = 0;
                return 1;
            }
        }
        at<std::uint16_t>(contact, 80) = 0;
    }
    return 0;
}

Byte locate_ground(void *contact, float *position, float *height) {
    if (sample_ground(position, contact)) {
        const float *plane = static_cast<const float *>(contact);
        auto E = [](float value) { return static_cast<long double>(value); };
        const auto z_slope = (E(position[2]) - plane[2]) * plane[5];
        const auto x_slope = (E(position[0]) - plane[0]) * plane[3];
        *height = float(E(position[1]) - (E(plane[1]) - (z_slope + x_slope) / E(plane[4])));
        return 1;
    }
    *height = 0;
    return 0;
}
} // namespace nfs::game::runtime::physics
namespace nfs::game::runtime::track {

void path_direction(void *path, int index, float *output) {
    const float *point =
        reinterpret_cast<const float *>(at<const Byte *>(path, 40) + Dword(index) * 32);
    if (at<int>(path, 36) < 2)
        output[0] = output[1] = output[2] = 0;
    else {
        if (index < at<int>(path, 36) - 1)
            for (unsigned axis = 0; axis < 3; ++axis)
                output[axis] = float(static_cast<long double>(point[8 + axis]) - point[axis]);
        else
            for (unsigned axis = 0; axis < 3; ++axis)
                output[axis] = float(static_cast<long double>(point[axis]) - point[static_cast<int>(axis) - 8]);
    }
    if (output[0] != 0 || output[1] != 0 || output[2] != 0)
        normalize_vector(output, output);
}
namespace x87 {
namespace {

long double direction_value(void *vehicle) {
    auto E = [](float value) { return static_cast<long double>(value); };
    void *path = invoke<void *>(vehicle, 1);
    const int index = invoke<int>(vehicle, 2);
    if (!path || index < 0)
        return E(1);
    float direction[3];
    path_direction(path, index, direction);
    void *base = bytes(vehicle) + at<int>(at<void *>(vehicle, 4), 12) + 4;
    const float *velocity = reinterpret_cast<const float *>(bytes(invoke<void *>(base, 0)) + 8);
    const auto result = (E(direction[1]) * velocity[1] + E(direction[0]) * velocity[0]) -
                        E(direction[2]) * velocity[2];
    return at<int>(vehicle, 240) == -1 ? -result : result;
}
} // namespace
void vehicle_direction_native80(void *vehicle, void *output) {
    physics::motion_x87::store_native80(direction_value(vehicle), output);
}
} // namespace x87
} // namespace nfs::game::runtime::track
