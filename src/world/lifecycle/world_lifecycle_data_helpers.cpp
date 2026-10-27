#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world {

void notify_camera_listeners(void *owner) {
    for (unsigned field = 80; field <= 100; field += 4) {
        if (void *listener = at<void *>(owner, field))
            invoke(listener, 2, offset(owner, 4));
    }
}

void begin_camera_mode(void *owner, Dword identifier) {
    int index = 0;
    for (; index < at<int>(owner, 8); ++index) {
        if (at<Dword>(at<void **>(owner, 4)[index], 104) == identifier)
            break;
    }
    int active = at<int>(owner, 16);
    if (active <= index) {
        void **controllers = at<void **>(owner, 4);
        invoke(controllers[index], 0);
        void *saved = controllers[active];
        controllers[active] = controllers[index];
        at<void **>(owner, 4)[index] = saved;
        active = at<int>(owner, 16);
        notify_camera_listeners(at<void **>(owner, 4)[active]);
        at<int>(owner, 16) = active + 1;
    }
}

void end_camera_mode(void *owner, Dword identifier) {
    int active = at<int>(owner, 16);
    int index = 0;
    for (; index < active; ++index) {
        if (at<Dword>(at<void **>(owner, 4)[index], 104) == identifier)
            break;
    }
    if (index < active) {
        void **controllers = at<void **>(owner, 4);
        void *controller = controllers[index];
        invoke(controller, 1);
        active = at<int>(owner, 16) - 1;
        at<int>(owner, 16) = active;
        void *saved = controllers[active];
        controllers[active] = controller;
        at<void **>(owner, 4)[index] = saved;
    }
}
// Exact callback is a single RET.
void __cdecl character_frame_callback(void *) {
}

void *list_node_at(void *list, int index) {
    void *node = at<void *>(list, 4);
    if (index)
        do {
            if (!node)
                return nullptr;
            --index;
            node = at<void *>(node, 12);
        } while (index);
    return node;
}

void *actor_at(void *list, int index) {
    void *node = list_node_at(list, index);
    return node ? static_cast<Byte *>(node) - 16 : nullptr;
}

void unlink_list_node(void *list, void *node) {
    if (!node)
        return;
    for (void *scan = at<void *>(list, 4); scan; scan = at<void *>(scan, 12)) {
    }
    void *next = at<void *>(node, 12);
    void *previous = at<void *>(node, 8);
    if (!next)
        at<void *>(list, 8) = previous;
    else
        at<void *>(next, 8) = previous;
    if (!previous)
        at<void *>(list, 4) = next;
    else
        at<void *>(previous, 12) = next;
    void *scan = at<void *>(list, 4);
    at<Dword>(node, 12) = at<Dword>(node, 8) = 0;
    at<Dword>(list, 12) -= 1;
    for (; scan; scan = at<void *>(scan, 12)) {
    }
}

void reset_actor_render_state(void *actor) {
    int count = at<int>(actor, 16);
    at<Byte>(actor, 8) = 1;
    if (count > 0) {
        Dword *entry = at<Dword *>(actor, 12) + 1;
        do {
            entry[1] = entry[0] = 0;
            entry += 5;
        } while (--count);
    }
}
// Exact body only initializes the first pointer.
void *construct_path_data(void *owner) {
    at<Dword>(owner, 0) = 0;
    return owner;
}

int count_path_locations(void *definitions) {
    int result = 0;
    Byte *marker = at<Byte *>(definitions, 60);
    for (int index = 0; index < at<int>(definitions, 0); ++index, marker += 36) {
        if (at<std::int16_t>(marker, 28) != 2 && at<Dword>(at<void *>(marker, 32), 16))
            ++result;
    }
    return result;
}

void *construct_path_location(void *owner) {
    at<Byte>(owner, 5) = 0;
    at<Byte>(owner, 4) = 0;
    at<Dword>(owner, 0) = 0;
    at<Dword>(owner, 8) = 0;
    at<Dword>(owner, 12) = 0;
    return owner;
}
// Confirmed empty destructor in the saved release image.
void destroy_path_location(void *) {
}
 
void finish_track_loading() {
}
// Word8 is deliberately not initialized by the constructor.
void *construct_gob_definitions(void *owner) {
    for (unsigned index = 0; index < 17; ++index)
        if (index != 8)
            at<Dword>(owner, index * 4) = 0;
    return owner;
}
 
void destroy_gob_definitions(void *) {
}

Dword *relocate_level_gob_blocks(void *owner, Dword *cursor) {
    if (!cursor)
        return nullptr;
    Dword *fields = static_cast<Dword *>(owner);
    int markers = int(*cursor++);
    fields[15] = reinterpret_cast<Dword>(cursor);
    cursor += markers * 9;
    int references = int(*cursor++);
    fields[1] = Dword(references);
    fields[14] = reinterpret_cast<Dword>(cursor);
    cursor += references * 5;
    constexpr unsigned count_slots[] = {2, 3, 4, 5, 6, 7, 8};
    constexpr unsigned pointer_slots[] = {9, 10, 11, 12, 13, 16, 17};
    constexpr unsigned strides[] = {30, 14, 17, 29, 4, 9, 3};
    for (unsigned block = 0; block < 7; ++block) {
        int count = int(*cursor++);
        fields[count_slots[block]] = Dword(count);
        fields[pointer_slots[block]] = reinterpret_cast<Dword>(cursor);
        if (block != 6)
            cursor += count * strides[block];
    }
    fields[0] = Dword(markers);
    for (int index = 0; index < markers; ++index) {
        auto *marker = reinterpret_cast<Dword *>(fields[15]) + index * 9;
        if (*reinterpret_cast<std::int16_t *>(marker + 7) != 2)
            marker[8] = marker[8] == 4294967295u ? 0 : fields[14] + marker[8] * 20;
    }
    for (int index = 0; index < references; ++index) {
        auto *entry = reinterpret_cast<Dword *>(fields[14]) + index * 5;
        constexpr unsigned bases[] = {9, 10, 11, 12, 13};
        constexpr unsigned sizes[] = {120, 56, 68, 116, 16};
        for (unsigned field = 0; field < 5; ++field)
            entry[field] =
                entry[field] == 4294967295u ? 0 : entry[field] * sizes[field] + fields[bases[field]];
    }
    int point_index = 0;
    for (int index = 0; index < int(fields[7]); ++index) {
        auto *marker = reinterpret_cast<Dword *>(fields[16]) + index * 9;
        marker[8] = fields[17] + Dword(point_index) * 12;
        point_index = int(Dword(point_index) + marker[5]);
    }
    return cursor + int(fields[8]) * 3;
}

void fix_level_points(void *owner) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    for (int index = 0, count = at<int>(owner, 116); index < count; ++index) {
        void *entry = offset(at<void *>(owner, 120), index * 140);
        if (at<Byte>(entry, 4) != 2)
            continue;
        float *points = reinterpret_cast<float *>(offset(entry, 52));
        volatile float minimum = points[1], maximum = points[1];
        for (unsigned vertex = 1; vertex < 4; ++vertex) {
            float value = points[vertex * 3 + 1];
            if ((value < minimum) != (std::isnan(value) || std::isnan(minimum)))
                minimum = value;
            if ((maximum < value) != (std::isnan(maximum) || std::isnan(value)))
                maximum = value;
        }
        float longest = 0;
        unsigned first = 0, second = 1;
        for (unsigned a = 0; a < 4; ++a) {
            for (unsigned b = a + 1; b < 4; ++b) {
                auto x = X(points[a * 3]) - points[b * 3];
                auto z = X(points[a * 3 + 2]) - points[b * 3 + 2];
                auto squared = z * z + x * x;
                if (X(longest) < squared) {
                    longest = float(squared);
                    first = a;
                    second = b;
                }
            }
        }
        Dword a[3], b[3];
        std::memcpy(a, points + first * 3, 12);
        std::memcpy(b, points + second * 3, 12);
        points[2] = -reinterpret_cast<float *>(a)[2];
        points[5] = -reinterpret_cast<float *>(b)[2];
        at<Dword>(entry, 52) = a[0];
        at<Dword>(entry, 56) = a[1];
        at<Dword>(entry, 64) = b[0];
        at<Dword>(entry, 68) = b[1];
    }
}

void construct_light_glow(void *object) {
    at<Dword>(object, 212) = 0;
    at<Dword>(object, 0) = 0;
    at<Byte>(object, 8) = 0;
    at<Dword>(object, 220) = 0;
    at<Byte>(object, 228) = 0;
    at<Byte>(object, 236) = 0;
    at<Byte>(object, 237) = 0;
    at<Dword>(object, 4) = 1065353216;
    at<Dword>(object, 240) = 0;
    at<Dword>(object, 244) = 1065353216;
}
// Spline basis , with explicit x87 stores.
void interpolate_path(int source_count, const float *points, int extra_segments, float *output) {
    const auto format = kernel::numeric::current_format();
    using kernel::numeric::Extended;
    auto X = [&](float value) { return Extended(value, format); };
    static constexpr float basis[16] = {-0.16666667163372039794921875f,
                                        0.5f,
                                        -0.5f,
                                        0.16666667163372039794921875f,
                                        0.5f,
                                        -1,
                                        0.5f,
                                        0,
                                        -0.5f,
                                        0,
                                        0.5f,
                                        0,
                                        0.16666667163372039794921875f,
                                        0.666666686534881591796875f,
                                        0.16666667163372039794921875f,
                                        0};
    float weights[3][4];
    Extended parameter = X(0);
    for (unsigned sample = 0; sample < 3; ++sample) {
        Extended squared = parameter * parameter;
        float powers[4] = {float(squared * parameter), float(squared), float(parameter), 1};
        for (unsigned column = 0; column < 4; ++column) {
            Extended value = X(0);
            for (unsigned row = 0; row < 4; ++row)
                value += X(powers[row]) * basis[row * 4 + column];
            weights[sample][column] = float(value);
        }
        parameter += 0.3333333432674407958984375f;
    }
    int total = int(Dword(source_count) + Dword(extra_segments));
    float *destination = output;
    for (int segment = 0; segment < total - 1; ++segment) {
        for (unsigned sample = 0; sample < 3; ++sample) {
            // The function reloads all four control points for every sample.
            float control[4][4];
            for (int corner = 0; corner < 4; ++corner) {
                int index = segment + corner;
                if (source_count <= index)
                    index = source_count - 1;
                control[corner][0] = points[index * 3];
                control[corner][1] = points[index * 3 + 1];
                control[corner][2] = points[index * 3 + 2];
                control[corner][3] = 1;
            }
            Extended values[3] = {X(0), X(0), X(0)};
            for (unsigned component = 0; component < 3; ++component) {
                values[component] = X(control[3][component]) * weights[sample][3];
                for (int corner = 2; corner >= 0; --corner)
                    values[component] += X(control[corner][component]) * weights[sample][corner];
            }
            float z = float(values[2]);
            destination[0] = float(values[0]);
            destination[1] = float(values[1]);
            destination[2] = -z;
            destination += 3;
        }
    }
    // This address is based on total, including the original gaps when total <= 1.
    output[total * 9 - 7] = -points[total * 3 - 1];
    output[total * 9 - 9] = points[total * 3 - 3];
    output[total * 9 - 8] = points[total * 3 - 2];
}

void read_replay_section(void *replay, int player, int *section, Byte *direction) {
    *section = at<int>(replay, 64 + player * 4);
    *direction = at<int>(replay, 56 + player * 4) != 0;
}
// Keep the five saved float stores separate from retained x87 values.
void quaternion_matrix(const float *quaternion, float *matrix) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    const auto x = X(quaternion[0]), y = X(quaternion[1]), z = X(quaternion[2]),
               w = X(quaternion[3]);
    const auto twice_xx = (x * x) + (x * x), twice_yy = (y * y) + (y * y),
               twice_zz = (z * z) + (z * z), twice_xy = (y * x) + (y * x);
    const float twice_xz = float((z * x) + (z * x)), twice_xw = float((w * x) + (w * x)),
                twice_yz = float((z * y) + (z * y)), twice_yw = float((w * y) + (w * y));
    const auto twice_zw = (w * z) + (w * z);
    matrix[0] = float((X(1.0f) - twice_yy) - twice_zz);
    matrix[1] = float(twice_xy - twice_zw);
    matrix[2] = float(X(twice_yw) + twice_xz);
    matrix[3] = float(twice_zw + twice_xy);
    const auto diagonal = X(1.0f) - twice_xx;
    const float stored_diagonal = float(diagonal);
    matrix[4] = float(diagonal - twice_zz);
    matrix[5] = float(X(twice_yz) - twice_xw);
    matrix[6] = float(X(twice_xz) - twice_yw);
    matrix[7] = float(X(twice_yz) + twice_xw);
    matrix[8] = float(X(stored_diagonal) - twice_yy);
}
} // namespace nfs::game::world
