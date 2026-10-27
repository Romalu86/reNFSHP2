#include "nfs/physics/contacts/contact_dependencies.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::physics::contact {

void *polygon_at(Dword *grid, std::uint16_t index) {
    return bytes(at<void *>(at<void *>(grid, 12), 4)) + unsigned(index) * 64;
}

void polygon_corners(void *polygon, float **output) {
    output[1] = reinterpret_cast<float *>(bytes(polygon) + 12);
    output[0] = static_cast<float *>(polygon);
    output[2] = reinterpret_cast<float *>(bytes(polygon) + 24);
    output[3] = reinterpret_cast<float *>(bytes(polygon) + 36);
}

void decode_normal(const void *polygon, float *output) {
    std::int16_t components[3];
    for (unsigned axis = 0; axis < 3; ++axis) {
        std::memcpy(components + axis, bytes(polygon) + 52 + axis * 2,
                    sizeof(components[axis]));
    }
    for (unsigned axis = 0; axis < 3; ++axis)
        output[axis] = float(static_cast<long double>(int(components[axis]) * 2) * 0.0000152587890625f);
}
namespace {
void clear_ground(void *contact) {
    // Initialize all six vectors from the zero triplet.
    for (unsigned index = 0; index < 19; ++index)
        at<Dword>(contact, index * 4) = 0;
    at<std::uint16_t>(contact, 76) = 0;
    at<std::uint16_t>(contact, 78) = 0;
    at<std::uint16_t>(contact, 80) = 0;
}
bool inside_edge(const float *point, const float *first, const float *second) {
    auto X = [](float value) { return static_cast<long double>(value); };
    const auto a = (X(first[0]) - second[0]) * (X(point[2]) - second[2]);
    const auto b = (X(first[2]) - second[2]) * (X(point[0]) - second[0]);
    return a <= b;
}
} // namespace

Byte search_ground(void *cell, void *compartment, const float *position, void *contact,
                   Byte surface) {
    if (!cell) {
        clear_ground(contact);
        return 0;
    }
    const unsigned count = at<std::uint16_t>(cell, 0);
    const auto *indices = at<const std::uint16_t *>(cell, 4);
    bool found = false;
    float ceiling;
    if (count)
        ceiling = float(static_cast<long double>(position[1]) + 2.0f);
    for (unsigned index = 0; index < count; ++index) {
        float *corners[4];
        void *polygon = track::polygon_vertices(static_cast<Dword *>(compartment), indices + index,
                                                &corners[0], &corners[1], &corners[2], &corners[3]);
        const Byte material = at<Byte>(polygon, 59) & 15;
        if (!(corners[0][1] < ceiling) || (surface && material && surface != material))
            continue;
        if (!inside_edge(position, corners[0], corners[1]) ||
            !inside_edge(position, corners[1], corners[2]) ||
            !inside_edge(position, corners[2], corners[3]) ||
            !inside_edge(position, corners[3], corners[0]))
            continue;
        const Byte classification = at<Byte>(polygon, 58), flags = at<Byte>(polygon, 59);
        at<std::uint16_t>(contact, 78) = classification;
        at<void *>(contact, 24) = polygon;
        at<std::uint16_t>(contact, 76) = flags >> 4;
        decode_normal(polygon, reinterpret_cast<float *>(bytes(contact) + 12));
        const auto lower_height = static_cast<long double>(position[1]) - 2.f;
        auto copy_corner = [](void *destination, const float *source) {
            at<Dword>(destination, 0) = at<Dword>(source, 0);
            const Dword y = at<Dword>(source, 4), z = at<Dword>(source, 8);
            at<Dword>(destination, 4) = y;
            at<Dword>(destination, 8) = z;
        };
        copy_corner(contact, corners[0]);
        copy_corner(bytes(contact) + 28, corners[0]);
        copy_corner(bytes(contact) + 40, corners[1]);
        copy_corner(bytes(contact) + 52, corners[2]);
        const Dword x = at<Dword>(corners[3], 0);
        const bool within_height = lower_height < static_cast<long double>(corners[0][1]);
        at<Dword>(contact, 64) = x;
        const Dword y = at<Dword>(corners[3], 4), z = at<Dword>(corners[3], 8);
        at<Dword>(contact, 68) = y;
        at<Dword>(contact, 72) = z;
        if (within_height)
            return 1;
        found = true;
    }
    if (found)
        return 1;
    clear_ground(contact);
    return 0;
}
} // namespace nfs::game::runtime::physics::contact
