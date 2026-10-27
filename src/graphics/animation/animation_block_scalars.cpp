#include "animation_channel_values.hpp"
#include <utility>
namespace nfs::game::world::actor_resources::animation_runtime {
Byte reset_backward_animation_blocks = 1;
namespace {
bool selected(const Dword *mask, unsigned bone) {
    return (mask[bone >> 5] & (Dword(1) << (bone & 31))) != 0;
}
int scalar_frame(void *owner, float time) {
    auto resource = at<void *>(owner, 12);
    int frame = channel_frame(time);
    auto keys = at<const std::uint16_t *>(resource, 8);
    int count = at<std::uint16_t>(resource, 12);
    if (!keys) {
        if (frame < 0)
            return 0;
        return frame < count ? frame : count - 1;
    }
    if (frame < keys[0])
        return 0;
    int index = at<int>(owner, 16) < 1 ? 0 : at<int>(owner, 16) - 1;
    if (frame < keys[index]) {
        while (index > 0 && frame < keys[index])
            --index;
    } else {
        while (index < count - 2 && !(frame < keys[index + 1]))
            ++index;
    }
    return index + 1;
}
Byte *scalar_block(void *resource, int block) {
    Dword count = at<std::uint16_t>(resource, 14);
    unsigned shift = at<Byte>(resource, 16) & 31;
    Dword stride = (((Dword(1) << shift) + 1) * count + 1) & ~Dword(1);
    return offset(resource, 20 + count * 12 + stride * Dword(block));
}
void copy_constant_scalars(void *resource, float *output) {
    Dword count = at<std::uint16_t>(resource, 14);
    Dword frames = at<std::uint16_t>(resource, 12);
    unsigned shift = at<Byte>(resource, 16) & 31;
    Dword remainder = frames & (2147483647u >> ((31 - shift) & 31));
    auto tail = scalar_block(resource, int(frames >> shift));
    if (remainder)
        tail = reinterpret_cast<Byte *>((Dword(tail) + (remainder + 1) * count + 1) & ~Dword(1));
    if (!count)
        tail = reinterpret_cast<Byte *>((Dword(tail) + 1) & ~Dword(1));
    Dword constants = at<Byte>(resource, 17);
    auto indices = reinterpret_cast<const std::uint16_t *>(tail);
    auto values = reinterpret_cast<const Dword *>((Dword(tail) + constants * 2 + 3) & ~Dword(3));
    for (Dword i = 0; i < constants; ++i)
        reinterpret_cast<Dword *>(output)[indices[i]] = values[i];
}

// Unmasked path's extra reset at an exact block boundary, as in the code.
Dword sample_scalar_blocks(void *owner, float time, float *output, const Dword *mask) {
    auto resource = at<void *>(owner, 12);
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    Dword mask_key = Dword(mask);
    if (at<Dword>(owner, 40) != mask_key) {
        at<int>(owner, 16) = -1;
        at<int>(owner, 28) = -1;
        at<Dword>(owner, 40) = mask_key;
    }
    int rounded = channel_frame(time), frame = scalar_frame(owner, time);
    unsigned shift = at<Byte>(resource, 16) & 31;
    Dword low_mask = 2147483647u >> ((31 - shift) & 31);
    int block_index = frame >> shift;
    Dword local_frame = Dword(frame) & low_mask;
    auto block = scalar_block(resource, block_index);
    Dword count = at<std::uint16_t>(resource, 14);
    auto indices = at<const std::uint16_t *>(resource, 4);
    Byte bones[80];
    if (mask)
        for (Dword i = 0; i < count; ++i)
            bones[i] = Byte(indices[i] / 12);
    auto enabled = [&](Dword i) { return !mask || selected(mask, bones[i]); };
    int cached = at<int>(owner, 16);
    bool reset_backward = frame < cached && reset_backward_animation_blocks == 0;
    if (frame == at<int>(owner, 28)) {
        std::swap(at<float *>(owner, 24), at<float *>(owner, 36));
        at<int>(owner, 16) = at<int>(owner, 28);
        at<int>(owner, 28) = -1;
    } else {
        Dword cursor;
        if (cached == -1 || (cached >> shift) != block_index || (!mask && local_frame == 0) ||
            reset_backward) {
            for (Dword i = 0; i < count; ++i) {
                if (!enabled(i))
                    continue;
                auto coefficients = at<const float *>(owner, 44) + i * 4;
                at<float *>(owner, 24)[i] = float(
                    Extended::integer(at<std::uint16_t>(block, i * 2), format) * coefficients[1] +
                    coefficients[0]);
            }
            cursor = 0;
        } else
            cursor = Dword(cached) & low_mask;
        if (int(cursor) < int(local_frame)) {
            auto deltas = block + (cursor + 2) * count;
            do {
                for (Dword i = 0; i < at<std::uint16_t>(resource, 14); ++i) {
                    if (!enabled(i))
                        continue;
                    auto coefficients = at<const float *>(owner, 44) + i * 4;
                    auto value = at<float *>(owner, 24) + i;
                    *value = float(Extended::integer(deltas[i], format) * coefficients[3] +
                                   coefficients[2] + *value);
                }
                deltas += count;
            } while (++cursor != local_frame);
        } else if (int(local_frame) < int(cursor)) {
            auto deltas = block + (cursor + 1) * count;
            do {
                for (Dword i = 0; i < at<std::uint16_t>(resource, 14); ++i) {
                    if (!enabled(i))
                        continue;
                    auto coefficients = at<const float *>(owner, 44) + i * 4;
                    auto value = at<float *>(owner, 24) + i;
                    *value =
                        float(X(*value) - (Extended::integer(deltas[i], format) * coefficients[3] +
                                           coefficients[2]));
                }
                deltas -= count;
            } while (--cursor != local_frame);
        }
    }
    at<int>(owner, 16) = frame;
    auto keys = at<const std::uint16_t *>(resource, 8);
    float fraction = 1;
    float start = !keys ? float(rounded) : frame ? float(keys[frame - 1]) : 0.0f;
    bool interpolate = !std::isnan(time) && time != start;
    if (interpolate) {
        if (!keys)
            fraction = float(X(time) - start);
        else if (!frame)
            fraction = float(X(time) / Extended::integer(keys[0], format));
        else
            fraction = float((X(time) - start) / (Extended::integer(keys[frame], format) - start));
    }
    if (interpolate && frame < int(at<std::uint16_t>(resource, 12)) - 1) {
        int next = frame + 1;
        int next_block = next >> shift;
        bool retained = at<int>(owner, 28) == next;
        for (Dword i = 0; i < at<std::uint16_t>(resource, 14); ++i) {
            if (!enabled(i))
                continue;
            auto current = at<float *>(owner, 24) + i;
            auto following = at<float *>(owner, 36) + i;
            if (retained)
                output[indices[i]] = float((X(*following) - *current) * fraction + *current);
            else if (next_block == block_index) {
                auto coefficients = at<const float *>(owner, 44) + i * 4;
                float delta =
                    float(Extended::integer(block[(local_frame + 2) * count + i], format) *
                              coefficients[3] +
                          coefficients[2]);
                *following = float(X(delta) + *current);
                output[indices[i]] = float(X(delta) * fraction + *current);
            } else {
                auto coefficients = at<const float *>(owner, 44) + i * 4;
                *following = float(
                    Extended::integer(at<std::uint16_t>(scalar_block(resource, next_block), i * 2),
                                      format) *
                        coefficients[1] +
                    coefficients[0]);
                output[indices[i]] = float((X(*following) - *current) * fraction + *current);
            }
        }
        if (!retained)
            at<int>(owner, 28) = next;
    } else {
        for (Dword i = 0; i < at<std::uint16_t>(resource, 14); ++i)
            if (enabled(i))
                reinterpret_cast<Dword *>(output)[indices[i]] =
                    reinterpret_cast<Dword *>(at<float *>(owner, 24))[i];
    }
    if (at<Byte>(resource, 17))
        copy_constant_scalars(resource, output);
    return 1; // Only the low byte of the result is used.
}
} // namespace

void allocate_scalar_block_buffers(void *owner) {
    auto resource = at<void *>(owner, 12);
    Dword count = at<std::uint16_t>(resource, 14);
    if (!count)
        return;
    at<void *>(owner, 24) = at<void *>(owner, 20) = allocate_animation_buffer(count << 2);
    at<void *>(owner, 36) = at<void *>(owner, 32) = allocate_animation_buffer(count << 2);
    at<void *>(owner, 44) = allocate_animation_buffer(count << 4);
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    for (Dword i = 0; i < at<std::uint16_t>(resource, 14); ++i) {
        auto encoded = offset(resource, 20 + i * 12);
        auto coefficients = at<float *>(owner, 44) + i * 4;
        coefficients[0] = at<float>(encoded, 0);
        coefficients[1] = at<float>(encoded, 4);
        coefficients[1] = float(X(coefficients[1]) * 0.00001525902189314365386962890625f);
        auto value = Extended::integer(at<std::uint16_t>(encoded, 8), format) *
                     at<float>(encoded, 4) * 0.00001525902189314365386962890625f;
        coefficients[2] = float(value + value - at<float>(encoded, 4));
        value = Extended::integer(at<std::uint16_t>(encoded, 10), format) * at<float>(encoded, 4) *
                0.00001525902189314365386962890625f;
        coefficients[3] = float(value + value);
        coefficients[3] = float(X(coefficients[3]) * 0.0039215688593685626983642578125f);
    }
}

Dword sample_masked_scalar_blocks(void *owner, float time, float *output, const Dword *mask) {
    return sample_scalar_blocks(owner, time, output, mask);
}

Dword __fastcall sample_block_scalar_animation(void *owner, void *, Dword time, Dword output,
                                              Dword mask) {
    if (!at<void *>(owner, 24))
        allocate_scalar_block_buffers(owner);
    return mask ? sample_masked_scalar_blocks(owner, channel_float(time),
                                              reinterpret_cast<float *>(output),
                                              reinterpret_cast<const Dword *>(mask))
                : sample_scalar_blocks(owner, channel_float(time),
                                       reinterpret_cast<float *>(output), nullptr);
}
} // namespace nfs::game::world::actor_resources::animation_runtime
