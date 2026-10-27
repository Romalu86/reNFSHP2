#include "animation_axis_rotation.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
namespace {
bool channel_selected(const Dword *mask, Byte bone) {
    return !mask || (mask[bone >> 5] & (Dword(1) << (bone & 31)));
}
void initialize_axis_bases(void *owner, void *resource) {
    Dword count = at<Byte>(resource, 6);
    at<Byte *>(owner, 20) = offset(resource, 16 + count * 14);
    at<Dword>(owner, 24) = (((Dword(1) << (at<Byte>(resource, 7) & 31)) + 1) * count + 1) & ~Dword(1);
    auto buffer = allocate_animation_buffer(count << 4);
    at<Dword *>(owner, 32) = at<Dword *>(owner, 36) = buffer;
    at<Byte *>(owner, 16) = offset(resource, 16);
    at<Dword *>(owner, 40) = allocate_animation_buffer(count << 4);
    at<Dword *>(owner, 44) = allocate_animation_buffer(count << 4);
    for (Dword channel = 0; channel < count; ++channel) {
        AxisRotationDescriptor descriptor;
        decode_axis_rotation_descriptor(at<const std::uint16_t *>(owner, 16) + channel * 7, nullptr,
                                        &descriptor);
        auto first = at<float *>(owner, 40) + channel * 4,
             second = at<float *>(owner, 44) + channel * 4;
        float angles[3];
        if (!descriptor.axis) {
            first[0] = first[1] = first[2] = 0;
            first[3] = 1;
            angles[0] = 0;
            angles[1] = descriptor.first_angle;
            angles[2] = descriptor.second_angle;
            quaternion_from_rotation_angles(angles, second);
        } else if (descriptor.axis == 1) {
            angles[0] = descriptor.first_angle;
            angles[1] = angles[2] = 0;
            quaternion_from_rotation_angles(angles, first);
            angles[0] = angles[1] = 0;
            angles[2] = descriptor.second_angle;
            quaternion_from_rotation_angles(angles, second);
        } else {
            angles[0] = descriptor.first_angle;
            angles[1] = descriptor.second_angle;
            angles[2] = 0;
            quaternion_from_rotation_angles(angles, first);
            second[0] = second[1] = second[2] = 0;
            second[3] = 1;
        }
    }
}
int axis_frame(void *owner, void *resource, int rounded) {
    int frames = at<std::uint16_t>(resource, 4);
    auto keys = at<const std::uint16_t *>(resource, 8);
    if (!keys)
        return rounded < 0 ? 0 : rounded < frames ? rounded : frames - 1;
    if (rounded < keys[0])
        return 0;
    int index = at<int>(owner, 28) < 1 ? 0 : at<int>(owner, 28) - 1;
    if (rounded < keys[index]) {
        while (index > 0 && rounded < keys[index])
            --index;
    } else {
        while (index < frames - 2 && !(rounded < keys[index + 1]))
            ++index;
    }
    return index + 1;
}
struct AxisValues {
    float x, y;
    Extended z;
    float w;
};
AxisValues absolute_values(const std::uint16_t *descriptor, const Byte *encoded) {
    auto format = kernel::numeric::current_format();
    auto unit = Extended(0.0039215688593685626983642578125f, format) + 0.0039215688593685626983642578125f;
    auto value = Extended::integer(encoded[0], format) * unit - 1.0f;
    auto w = Extended::integer(encoded[1], format) * unit - 1.0f;
    std::int16_t axis = std::int16_t(descriptor[6]);
    return {axis == 0 ? float(value) : 0, axis == 1 ? float(value) : 0,
            axis == 0 || axis == 1 ? Extended(0, format) : value, float(w)};
}
AxisValues delta_values(const std::uint16_t *descriptor, Byte encoded) {
    auto format = kernel::numeric::current_format();
    auto I = [&](unsigned v) { return Extended::integer(v, format); };
    auto X = [&](float v) { return Extended(v, format); };
    auto unit = X(0.00001525902189314365386962890625f) + 0.00001525902189314365386962890625f;
    float first_offset = float(I(descriptor[2]) * unit - 1.0f),
          second_offset = float(I(descriptor[3]) * unit - 1.0f);
    float first_scale = float(I(descriptor[4]) * unit),
          second_scale = float(I(descriptor[5]) * unit);
    auto value = I(encoded >> 4) * first_scale * 0.066666670143604278564453125f + first_offset;
    float w = float(I(encoded & 15) * second_scale * 0.066666670143604278564453125f + second_offset);
    Byte axis = Byte(descriptor[6]);
    return {axis == 0 ? float(value) : 0, axis == 1 ? float(value) : 0,
            axis == 0 || axis == 1 ? X(0) : value, w};
}
void compose_axis_pose(void *owner, unsigned channel, const float *axis, float *out) {
    auto descriptor = at<const std::uint16_t *>(owner, 16) + channel * 7;
    auto first = at<const float *>(owner, 40) + channel * 4,
         second = at<const float *>(owner, 44) + channel * 4;
    auto mode = std::int16_t(descriptor[6]);
    if (!mode)
        compose_x_axis_rotation(axis, second, out);
    else if (mode == 1)
        compose_y_axis_rotation(first, axis, second, out);
    else
        compose_z_axis_rotation(first, axis, out);
}
void interpolate_axis_pose(const float *current, const AxisValues &next, float fraction,
                           float *out) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    float x = float((X(next.x) - current[0]) * fraction + current[0]);
    float y = float((X(next.y) - current[1]) * fraction + current[1]);
    auto z = (next.z - current[2]) * fraction + current[2];
    auto w = (X(next.w) - current[3]) * fraction + current[3];
    auto inverse = X(1) / (((w * w + z * z) + X(y) * y) + X(x) * x).square_root();
    out[0] = float(X(x) * inverse);
    out[1] = float(X(y) * inverse);
    out[2] = float(z * inverse);
    out[3] = float(w * inverse);
}
} // namespace
// Slot 18 accepts time, mask and pose arguments.
Dword __fastcall sample_axis_rotation_pose(void *owner, void *, Dword time_word, Dword mask_word,
                                          Dword pose_word) {
    if (!at<Dword>(owner, 36))
        initialize_axis_bases(owner, at<void *>(owner, 12));
    auto resource = at<void *>(owner, 12);
    auto indices = at<const Byte *>(resource, 12);
    auto mask = reinterpret_cast<const Dword *>(mask_word);
    auto pose = reinterpret_cast<Dword *>(pose_word);
    float time = channel_float(time_word);
    int rounded = channel_frame(time), frame = axis_frame(owner, resource, rounded);
    unsigned shift = at<Byte>(resource, 7) & 31;
    Dword low_mask = 2147483647u >> ((31 - shift) & 31), local_frame = Dword(frame) & low_mask;
    int block_index = frame >> shift, cached = at<int>(owner, 28);
    auto block = at<Byte *>(owner, 20) + at<Dword>(owner, 24) * Dword(block_index);
    Dword count = at<Byte>(resource, 6), cursor;
    auto cache = at<float *>(owner, 36);
    if (cached == -1 || (cached >> shift) != block_index || frame < cached) {
        for (Dword i = 0; i < count; ++i) {
            if (!channel_selected(mask, indices[i]))
                continue;
            auto values =
                absolute_values(at<const std::uint16_t *>(owner, 16) + i * 7, block + i * 2);
            cache[i * 4] = values.x;
            cache[i * 4 + 1] = values.y;
            cache[i * 4 + 2] = float(values.z);
            cache[i * 4 + 3] = values.w;
        }
        cursor = 0;
    } else
        cursor = Dword(cached) & low_mask;
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto encoded = block + (cursor + 2) * count;
    for (; int(cursor) < int(local_frame); ++cursor) {
        for (Dword i = 0; i < count; ++i, ++encoded) {
            if (!channel_selected(mask, indices[i]))
                continue;
            auto delta = delta_values(at<const std::uint16_t *>(owner, 16) + i * 7, *encoded);
            auto rotation = cache + i * 4;
            rotation[0] = float(X(delta.x) + rotation[0]);
            rotation[1] = float(X(delta.y) + rotation[1]);
            rotation[2] = float(delta.z + rotation[2]);
            rotation[3] = float(X(delta.w) + rotation[3]);
        }
    }
    at<int>(owner, 28) = frame;
    auto keys = at<const std::uint16_t *>(resource, 8);
    float start = !keys ? float(rounded) : frame ? float(keys[frame - 1]) : 0;
    bool interpolate = time != start;
    float fraction = time;
    if (interpolate) {
        if (!keys)
            fraction = float(X(time) - start);
        else if (!frame)
            fraction = float(X(time) / Extended::integer(keys[0], format));
        else
            fraction = float((X(time) - start) / (Extended::integer(keys[frame], format) - start));
    }
    interpolate = interpolate && frame < int(at<std::uint16_t>(resource, 4)) - 1;
    auto next_block = at<Byte *>(owner, 20) + at<Dword>(owner, 24) * Dword((frame + 1) >> shift);
    for (Dword i = 0; i < count; ++i) {
        if (!channel_selected(mask, indices[i]))
            continue;
        auto current = cache + i * 4;
        auto out = reinterpret_cast<float *>(pose) + Dword(indices[i]) * 12 + 4;
        if (!interpolate) {
            compose_axis_pose(owner, i, current, out);
            continue;
        }
        auto descriptor = at<const std::uint16_t *>(owner, 16) + i * 7;
        if (((frame + 1) >> shift) == block_index) {
            auto delta = delta_values(descriptor, next_block[(local_frame + 2) * count + i]);
            AxisValues next = {float(X(delta.x) + current[0]), float(X(delta.y) + current[1]),
                               delta.z + current[2], delta.w};
            // W remains retained through its add/subtract until interpolation in this branch.
            float normalized[4];
            float x = float((X(next.x) - current[0]) * fraction + current[0]);
            float y = float((X(next.y) - current[1]) * fraction + current[1]);
            float z = float((next.z - current[2]) * fraction + current[2]);
            auto w = ((X(delta.w) + current[3]) - current[3]) * fraction + current[3];
            auto inverse = X(1) / (((w * w + X(z) * z) + X(y) * y) + X(x) * x).square_root();
            normalized[0] = float(X(x) * inverse);
            normalized[1] = float(X(y) * inverse);
            normalized[2] = float(X(z) * inverse);
            normalized[3] = float(w * inverse);
            compose_axis_pose(owner, i, normalized, out);
        } else {
            AxisValues next = absolute_values(descriptor, next_block + i * 2);
            float normalized[4];
            interpolate_axis_pose(current, next, fraction, normalized);
            compose_axis_pose(owner, i, normalized, out);
        }
    }
    return 1;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
