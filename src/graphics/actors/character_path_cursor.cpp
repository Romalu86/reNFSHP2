#include "character_runtime.hpp"
#include <cstring>
namespace nfs::game::world::actor_resources {

void bind_path_cursor(void *cursor, void *path) {
    at<void *>(cursor, 0) = path;
    at<Byte>(cursor, 5) = 0;
}

void locate_path_cursor(void *cursor, const float *position) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    void *path = at<void *>(cursor, 0);
    const int limit = int(at<Dword>(path, 12) - 1);
    at<Byte>(cursor, 4) = 1;
    at<Byte>(cursor, 5) = 0;
    float closest = 1000000;
    int selected = 0;
    for (int index = 0; index < limit; ++index) {
        const float *point = at<float *>(path, 0) + Dword(index) * 3;
        const auto x = E(position[0]) - point[0], y = E(position[1]) - point[1],
                   z = E(position[2]) - point[2];
        const auto square = z * z + x * x + y * y;
        if (square < E(closest)) {
            closest = float(square);
            selected = index;
        }
    }
    const float distance = float(E(closest).square_root());
    at<int>(cursor, 8) = selected;
    if (distance <= .01f) {
        at<Byte>(cursor, 5) = 1;
        at<Dword>(cursor, 12) = 0;
    }
}

Dword advance_path_cursor(void *cursor, float *position, float distance, float *output,
                         float *direction) {
    if (!at<Byte>(cursor, 4))
        return 0;
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    const float height = position[1];
    void *path = at<void *>(cursor, 0);
    if (at<Byte>(cursor, 5)) {
        if (distance == 0)
            return 0;
        float length, fraction;
        for (;;) {
            const Dword index = at<Dword>(cursor, 8);
            length = at<float *>(path, 8)[index];
            fraction = float(E(distance) / E(length) + at<float>(cursor, 12));
            at<float>(cursor, 12) = fraction;
            if (!(fraction > 1))
                break;
            const Dword next = index + 1;
            distance = float((E(fraction) - 1.f) * length);
            at<Dword>(cursor, 12) = 0;
            at<Dword>(cursor, 8) = next;
            if (int(next) >= int(at<Dword>(path, 12) - 1)) {
                const bool loop = at<Dword>(path, 64) != 0;
                const Dword end = loop ? 0 : at<Dword>(path, 12) - 1;
                std::memcpy(output, at<float *>(path, 0) + end * 3, 12);
                std::memcpy(direction, at<float *>(path, 4) + end * 3, 12);
                output[1] = height;
                at<Dword>(cursor, 8) = end;
                if (at<Byte>(path, 68))
                    runtime::scale_vectors(1, direction, -1, direction);
                if (!loop)
                    at<Byte>(cursor, 4) = at<Byte>(cursor, 5) = 0;
                return 1;
            }
            std::memcpy(position, at<float *>(path, 0) + next * 3, 12);
            if (distance == 0)
                return 0;
        }
        const Dword index = at<Dword>(cursor, 8);
        const float *points = at<float *>(path, 0) + index * 3;
        float tangent[3] = {float(E(points[3]) - points[0]), 0, float(E(points[5]) - points[2])};
        runtime::normalize_vector(tangent, tangent);
        const float step = float(E(fraction) * length);
        runtime::scale_vectors(1, tangent, step, tangent);
        output[1] = height;
        output[0] = float(E(tangent[0]) + points[0]);
        output[2] = float(E(points[2]) + tangent[2]);
        const float *directions = at<float *>(path, 4) + index * 3;
        direction[0] = float(E(fraction) * directions[3] + E(directions[0]) * (E(1) - fraction));
        direction[1] = 0;
        direction[2] = float(E(fraction) * directions[5] + (E(1) - fraction) * directions[2]);
        runtime::normalize_vector(direction, direction);
    } else {
        const Dword index = at<Dword>(cursor, 8);
        const float *point = at<float *>(path, 0) + index * 3;
        const float target_x = point[0], target_z = point[2];
        float delta[3] = {float(E(target_x) - position[0]), 0, float(E(target_z) - position[2])};
        runtime::normalize_vector(delta, delta);
        runtime::scale_vectors(1, delta, distance, delta);
        const float old_x = position[0], old_z = position[2];
        output[1] = height;
        output[0] = float(E(delta[0]) + old_x);
        output[2] = float(E(delta[2]) + old_z);
        delta[0] = float(E(target_x) - (E(delta[0]) + old_x));
        delta[1] = 0;
        delta[2] = float(E(target_z) - (E(delta[2]) + old_z));
        const auto square = E(delta[0]) * delta[0] + E(delta[2]) * delta[2];
        if (!(square < E(distance) * distance))
            runtime::normalize_vector(delta, direction);
        else {
            std::memcpy(direction, at<float *>(path, 4) + index * 3, 12);
            at<Dword>(cursor, 12) = 0;
            at<Byte>(cursor, 5) = 1;
        }
    }
    if (at<Byte>(path, 68))
        runtime::scale_vectors(1, direction, -1, direction);
    return 0;
}
} // namespace nfs::game::world::actor_resources
