#include "animation_channel_values.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
extern Byte reset_backward_animation_blocks;
namespace {
bool enabled(const Dword *mask, Byte bone) {
    return !mask || (mask[bone >> 5] & (Dword(1) << (bone & 31)));
}
Byte *constant_indices(void *resource) {
    Dword length = Dword(1) << (at<Byte>(resource, 16) & 31);
    Dword count = at<Byte>(resource, 6), frames = at<std::uint16_t>(resource, 4);
    Dword remainder = frames % length;
    Dword stride = ((length + 1) * count * 3 + 1) & ~Dword(1);
    auto tail = offset(resource, 18 + count * 12 + stride * (frames / length));
    if (int(remainder) > 0)
        tail += (remainder + 1) * count * 3;
    return tail;
}
Extended complete_rotation(float *rotation, const Extended &square, bool negative) {
    auto format = kernel::numeric::current_format();
    Extended one(1, format);
    if (one < square) {
        auto inverse = one / square.square_root();
        rotation[3] = 0;
        for (unsigned i = 0; i < 3; ++i)
            rotation[i] = float(inverse * rotation[i]);
        return Extended(0, format);
    }
    auto w = (one - square).square_root();
    if (negative)
        w = -w;
    rotation[3] = float(w);
    return w;
}
Extended decode_rotation(float *rotation, const Byte *encoded, bool cache) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto I = [&](unsigned v) { return Extended::integer(v, format); };
    auto words = reinterpret_cast<const std::uint16_t *>(encoded);
    auto xy_unit = X(0.00001525902189314365386962890625f) + 0.00001525902189314365386962890625f;
    auto x_unit = X(0.000030518509447574615478515625f) + 0.000030518509447574615478515625f;
    rotation[0] = float(I(words[0] >> 1) * x_unit - 1.0f);
    rotation[1] = float(I(words[1]) * xy_unit - 1.0f);
    auto z = xy_unit * I(words[2]) - 1.0f;
    rotation[2] = float(z);
    auto square = cache ? X(rotation[0]) * rotation[0] + X(rotation[1]) * rotation[1] + z * z
                        : X(rotation[0]) * rotation[0] + X(rotation[2]) * rotation[2] +
                              X(rotation[1]) * rotation[1];
    return complete_rotation(rotation, square, (encoded[0] & 1) != 0);
}
struct RotationDelta {
    Extended x, y;
    float z;
};
RotationDelta rotation_delta(void *owner, unsigned channel, const Byte *encoded) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto I = [&](unsigned v) { return Extended::integer(v, format); };
    auto coefficients = at<const std::uint16_t *>(owner, 16) + channel * 6;
    auto unit = X(0.00001525902189314365386962890625f) + 0.00001525902189314365386962890625f;
    auto offset_x = I(coefficients[0]) * unit - 1.0f;
    float offset_y = float(I(coefficients[1]) * unit - 1.0f);
    float offset_z = float(I(coefficients[2]) * unit - 1.0f);
    auto scale_x = I(coefficients[3]) * unit;
    float scale_y = float(I(coefficients[4]) * unit), scale_z = float(I(coefficients[5]) * unit);
    return {I(encoded[0] >> 1) * scale_x * 0.0078740157186985015869140625f + offset_x,
            I(encoded[1]) * scale_y * 0.0039215688593685626983642578125f + offset_y,
            float(I(encoded[2]) * scale_z * 0.0039215688593685626983642578125f + offset_z)};
}
void apply_delta(float *out, const float *source, const RotationDelta &delta, bool forward) {
    auto format = kernel::numeric::current_format();
    out[0] = float(forward ? delta.x + source[0] : Extended(source[0], format) - delta.x);
    out[1] = float(forward ? delta.y + source[1] : Extended(source[1], format) - delta.y);
    out[2] = float(forward ? Extended(delta.z, format) + source[2]
                           : Extended(source[2], format) - delta.z);
}
void blend_rotation(float *out, const float *first, const float *second, const Extended &second_w,
                    const Extended &fraction) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto x = (X(second[0]) - first[0]) * fraction + first[0];
    float y = float((X(second[1]) - first[1]) * fraction + first[1]);
    float z = float((X(second[2]) - first[2]) * fraction + first[2]);
    auto w = (second_w - first[3]) * fraction + first[3];
    float stored_w = float(w);
    auto square = w * stored_w + X(z) * z + X(y) * y + x * x;
    auto inverse = X(1) / square.square_root();
    float stored_inverse = float(inverse);
    out[0] = float(inverse * x);
    out[1] = float(X(stored_inverse) * y);
    out[2] = float(X(stored_inverse) * z);
    out[3] = float(X(stored_inverse) * stored_w);
}
} // namespace

Byte *__fastcall find_rotation_constant_indices(void *resource, void *) {
    return constant_indices(resource);
}

Byte *__fastcall find_rotation_constant_values(void *resource, void *) {
    return reinterpret_cast<Byte *>((Dword(constant_indices(resource)) + at<Byte>(resource, 7) + 1) &
                                    ~Dword(1));
}
// Slot18 takes time, mask, pose; slot6 is the argument-reordering thunk.
Dword __fastcall sample_three_component_rotation_pose(void *owner, void *, Dword time_word,
                                                     Dword mask_word, Dword pose_word) {
    auto resource = at<void *>(owner, 12);
    if (!at<Dword>(owner, 20)) {
        at<Byte *>(owner, 20) = offset(resource, 18 + Dword(at<Byte>(resource, 6)) * 12);
        at<Byte *>(owner, 40) = find_rotation_constant_indices(resource, nullptr);
        at<Byte *>(owner, 44) = find_rotation_constant_values(resource, nullptr);
        at<Dword>(owner, 24) =
            (((Dword(1) << (at<Byte>(resource, 16) & 31)) + 1) * at<Byte>(resource, 6) * 3 + 1) &
            ~Dword(1);
        if (at<Byte>(resource, 6)) {
            auto buffer = allocate_animation_buffer(Dword(at<Byte>(resource, 6)) << 4);
            at<Dword *>(owner, 32) = at<Dword *>(owner, 36) = buffer;
            at<Byte *>(owner, 16) = offset(resource, 18);
        }
    }
    resource = at<void *>(owner, 12);
    auto mask = reinterpret_cast<const Dword *>(mask_word);
    auto pose = reinterpret_cast<Byte *>(pose_word);
    auto indices = at<const Byte *>(resource, 12);
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    float time = channel_float(time_word);
    if (at<Byte>(resource, 6)) {
        int rounded = channel_frame(time), frame;
        int frames = at<std::uint16_t>(resource, 4);
        auto keys = at<const std::uint16_t *>(resource, 8);
        if (!keys)
            frame = rounded < 0 ? 0 : rounded < frames ? rounded : frames - 1;
        else if (rounded < keys[0])
            frame = 0;
        else {
            int index = at<int>(owner, 28) < 1 ? 0 : at<int>(owner, 28) - 1;
            if (rounded < keys[index]) {
                while (index > 0 && rounded < keys[index])
                    --index;
            } else {
                while (index < frames - 2 && !(rounded < keys[index + 1]))
                    ++index;
            }
            frame = index + 1;
        }
        unsigned shift = at<Byte>(resource, 16) & 31;
        Dword low_mask = 2147483647u >> ((31 - shift) & 31);
        Dword local_frame = Dword(frame) & low_mask;
        int block_index = frame >> shift, cached = at<int>(owner, 28);
        auto block = at<Byte *>(owner, 20) + at<Dword>(owner, 24) * Dword(block_index);
        Dword count = at<Byte>(resource, 6), cursor;
        auto cache = at<float *>(owner, 36);
        if (cached == -1 || (cached >> shift) != block_index || !local_frame ||
            (frame < cached && !reset_backward_animation_blocks)) {
            for (unsigned i = 0; i < count; ++i)
                if (enabled(mask, indices[i]))
                    decode_rotation(cache + i * 4, block + i * 6, true);
            cursor = 0;
        } else
            cursor = Dword(cached) & low_mask;
        bool forward = int(cursor) < int(local_frame);
        bool moved = cursor != local_frame;
        auto encoded = block + (cursor + 2) * count * 3 - (forward ? 0 : 3);
        while (cursor != local_frame) {
            if (forward) {
                for (unsigned i = 0; i < count; ++i, encoded += 3)
                    if (enabled(mask, indices[i]))
                        apply_delta(cache + i * 4, cache + i * 4, rotation_delta(owner, i, encoded),
                                    true);
                ++cursor;
            } else {
                for (int i = int(count) - 1; i >= 0; --i, encoded -= 3)
                    if (enabled(mask, indices[i]))
                        apply_delta(cache + i * 4, cache + i * 4,
                                    rotation_delta(owner, unsigned(i), encoded), false);
                --cursor;
            }
        }
        if (moved) {
            encoded = block + (local_frame + 1) * count * 3;
            for (unsigned i = 0; i < count; ++i, encoded += 3) {
                if (!enabled(mask, indices[i]))
                    continue;
                auto rotation = cache + i * 4;
                auto square = X(rotation[0]) * rotation[0] + X(rotation[1]) * rotation[1] +
                              X(rotation[2]) * rotation[2];
                complete_rotation(rotation, square, (encoded[0] & 1) != 0);
            }
        }
        at<int>(owner, 28) = frame;
        auto start = !keys   ? Extended::integer(rounded, format)
                     : frame ? Extended::integer(keys[frame - 1], format)
                             : X(0);
        bool interpolate = std::isnan(time) || X(time) < start || start < X(time);
        auto fraction = X(time);
        if (interpolate) {
            if (!keys)
                fraction = X(time) - start;
            else if (!frame)
                fraction = X(time) / Extended::integer(keys[0], format);
            else
                fraction = (X(time) - start) / (Extended::integer(keys[frame], format) - start);
        }
        interpolate = interpolate && frame < frames - 1;
        auto next_block = at<Byte *>(owner, 20) + at<Dword>(owner, 24) * Dword((frame + 1) >> shift);
        for (unsigned i = 0; i < count; ++i) {
            if (!enabled(mask, indices[i]))
                continue;
            auto out = reinterpret_cast<float *>(pose + Dword(indices[i]) * 48 + 16);
            auto current = cache + i * 4;
            if (!interpolate) {
                std::memcpy(out, current, 16);
                continue;
            }
            float next[4];
            auto w = X(0);
            if (((frame + 1) >> shift) == block_index) {
                auto delta_bytes = next_block + (local_frame + 2) * count * 3 + i * 3;
                auto delta = rotation_delta(owner, i, delta_bytes);
                next[0] = float(delta.x + current[0]);
                next[1] = float(delta.y + current[1]);
                auto z = X(delta.z) + current[2];
                next[2] = float(z);
                auto square = z * next[2] + X(next[1]) * next[1] + X(next[0]) * next[0];
                w = complete_rotation(next, square, (delta_bytes[0] & 1) != 0);
            } else
                w = decode_rotation(next, next_block + i * 6, false);
            blend_rotation(out, current, next, w, fraction);
        }
    }
    for (unsigned i = 0; i < at<Byte>(resource, 7); ++i) {
        // The saved constant-channel mask uses the animated index array.
        if (!enabled(mask, indices[i]))
            continue;
        auto bone = at<const Byte *>(owner, 40)[i];
        decode_rotation(reinterpret_cast<float *>(pose + Dword(bone) * 48 + 16),
                        at<const Byte *>(owner, 44) + i * 6, true);
    }
    if (mask)
        at<int>(owner, 28) = -1;
    return 1;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
