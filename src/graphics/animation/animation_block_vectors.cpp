#include "animation_channel_values.hpp"
#include <utility>
namespace nfs::game::world::actor_resources::animation_runtime {
extern Byte reset_backward_animation_blocks;
namespace {
int vector_frame(void *owner, float time) {
    auto resource = at<void *>(owner, 12);
    int frame = channel_frame(time), count = at<std::uint16_t>(resource, 12);
    auto keys = at<const std::uint16_t *>(resource, 8);
    if (!keys)
        return frame < 0 ? 0 : frame < count ? frame : count - 1;
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
Byte *vector_block(void *resource, int block) {
    Dword count = at<std::uint16_t>(resource, 14);
    Dword length = Dword(1) << (at<Byte>(resource, 16) & 31);
    return offset(resource,
                  20 + count * 36 + (((length + 1) * count * 3 + 1) & ~Dword(1)) * Dword(block));
}
void constant_vectors(void *resource, float *output) {
    Dword count = at<std::uint16_t>(resource, 14), frames = at<std::uint16_t>(resource, 12);
    unsigned shift = at<Byte>(resource, 16) & 31;
    Dword remainder = frames & (2147483647u >> ((31 - shift) & 31));
    auto tail = vector_block(resource, int(frames >> shift));
    if (remainder)
        tail = reinterpret_cast<Byte *>((Dword(tail) + (remainder + 1) * count * 3 + 1) & ~Dword(1));
    if (!count)
        tail = reinterpret_cast<Byte *>((Dword(tail) + 1) & ~Dword(1));
    Dword constants = at<Byte>(resource, 17);
    auto indices = reinterpret_cast<const std::uint16_t *>(tail);
    auto values = reinterpret_cast<const Dword *>((Dword(tail) + constants * 2 + 3) & ~Dword(3));
    for (Dword i = 0; i < constants; ++i) {
        auto destination = reinterpret_cast<Dword *>(output) + indices[i];
        destination[0] = values[i * 3];
        destination[1] = values[i * 3 + 1];
        destination[2] = values[i * 3 + 2];
    }
}
Dword sample_vector_blocks(void *owner, float time, float *output, const Dword *mask) {
    auto resource = at<void *>(owner, 12);
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    if (at<Dword>(owner, 40) != Dword(mask)) {
        at<int>(owner, 16) = -1;
        at<int>(owner, 28) = -1;
        at<Dword>(owner, 40) = Dword(mask);
    }
    int frame = vector_frame(owner, time), rounded = channel_frame(time);
    unsigned shift = at<Byte>(resource, 16) & 31;
    Dword low_mask = 2147483647u >> ((31 - shift) & 31), local_frame = Dword(frame) & low_mask;
    int block_index = frame >> shift;
    Dword count = at<std::uint16_t>(resource, 14);
    auto indices = at<const std::uint16_t *>(resource, 4);
    Byte bones[80];
    if (mask)
        for (Dword i = 0; i < count; ++i)
            bones[i] = Byte(indices[i] / 12);
    auto enabled = [&](Dword i) {
        return !mask || (mask[bones[i] >> 5] & (Dword(1) << (bones[i] & 31)));
    };
    auto block = vector_block(resource, block_index);
    int cached = at<int>(owner, 16);
    bool backward = frame < cached && reset_backward_animation_blocks == 0;
    if (frame == at<int>(owner, 28)) {
        std::swap(at<float *>(owner, 24), at<float *>(owner, 36));
        at<int>(owner, 16) = at<int>(owner, 28);
        at<int>(owner, 28) = -1;
    } else {
        Dword cursor;
        if (cached == -1 || (cached >> shift) != block_index || (!mask && !local_frame) ||
            backward) {
            for (Dword i = 0; i < at<std::uint16_t>(resource, 14); ++i) {
                if (!enabled(i))
                    continue;
                auto coefficients = at<const float *>(owner, 44) + i * 12;
                for (unsigned axis = 0; axis < 3; ++axis)
                    at<float *>(owner, 24)[i * 4 + axis] = float(
                        Extended::integer(at<std::uint16_t>(block, (i * 3 + axis) * 2), format) *
                            coefficients[3 + axis] +
                        coefficients[axis]);
            }
            cursor = 0;
        } else
            cursor = Dword(cached) & low_mask;
        bool forward = int(cursor) < int(local_frame);
        if (cursor != local_frame) {
            auto deltas = block + (cursor + (forward ? 2 : 1)) * count * 3;
            do {
                for (Dword i = 0; i < at<std::uint16_t>(resource, 14); ++i) {
                    if (!enabled(i))
                        continue;
                    auto coefficients = at<const float *>(owner, 44) + i * 12;
                    auto destination = at<float *>(owner, 24) + i * 4;
                    auto x = Extended::integer(deltas[i * 3], format) * coefficients[9] +
                             coefficients[6];
                    auto y = Extended::integer(deltas[i * 3 + 1], format) * coefficients[10] +
                             coefficients[7];
                    float z =
                        float(Extended::integer(deltas[i * 3 + 2], format) * coefficients[11] +
                              coefficients[8]);
                    destination[0] = float(forward ? x + destination[0] : X(destination[0]) - x);
                    destination[1] = float(forward ? y + destination[1] : X(destination[1]) - y);
                    destination[2] = float(forward ? X(z) + destination[2] : X(destination[2]) - z);
                }
                if (forward) {
                    ++cursor;
                    deltas += count * 3;
                } else {
                    --cursor;
                    deltas -= count * 3;
                }
            } while (cursor != local_frame);
        }
    }
    at<int>(owner, 16) = frame;
    auto keys = at<const std::uint16_t *>(resource, 8);
    float start = !keys ? float(rounded) : frame ? float(keys[frame - 1]) : 0.0f;
    bool interpolate = !std::isnan(time) && time != start;
    float fraction = 1;
    if (interpolate) {
        if (!keys)
            fraction = float(X(time) - start);
        else if (!frame)
            fraction = float(X(time) / Extended::integer(keys[0], format));
        else
            fraction = float((X(time) - start) / (Extended::integer(keys[frame], format) - start));
    }
    if (interpolate && frame < int(at<std::uint16_t>(resource, 12)) - 1) {
        int next = frame + 1, next_block = next >> shift;
        bool retained = at<int>(owner, 28) == next;
        for (Dword i = 0; i < at<std::uint16_t>(resource, 14); ++i) {
            if (!enabled(i))
                continue;
            auto current = at<float *>(owner, 24) + i * 4;
            auto following = at<float *>(owner, 36) + i * 4;
            if (retained) {
                for (unsigned axis = 0; axis < 3; ++axis)
                    output[indices[i] + axis] =
                        float((X(following[axis]) - current[axis]) * fraction + current[axis]);
            } else if (next_block == block_index) {
                auto coefficients = at<const float *>(owner, 44) + i * 12;
                auto encoded = block + (local_frame + 2) * count * 3 + i * 3;
                auto x = Extended::integer(encoded[0], format) * coefficients[9] + coefficients[6];
                auto y = Extended::integer(encoded[1], format) * coefficients[10] + coefficients[7];
                float z = float(Extended::integer(encoded[2], format) * coefficients[11] +
                                coefficients[8]);
                if (mask)
                    at<int>(owner, 28) = next;
                following[0] = float(x + current[0]);
                following[1] = float(y + current[1]);
                following[2] = float(X(z) + current[2]);
                output[indices[i]] = float(x * fraction + current[0]);
                output[indices[i] + 1] = float(y * fraction + current[1]);
                output[indices[i] + 2] = float(X(z) * fraction + current[2]);
            } else {
                auto coefficients = at<const float *>(owner, 44) + i * 12;
                for (unsigned axis = 0; axis < 3; ++axis)
                    following[axis] = float(
                        Extended::integer(at<std::uint16_t>(vector_block(resource, next_block),
                                                            (i * 3 + axis) * 2),
                                          format) *
                            coefficients[axis + 3] +
                        coefficients[axis]);
                if (mask)
                    at<int>(owner, 28) = next;
                for (unsigned axis = 0; axis < 3; ++axis)
                    output[indices[i] + axis] =
                        float((X(following[axis]) - current[axis]) * fraction + current[axis]);
            }
        }
        if (!mask && !retained)
            at<int>(owner, 28) = next;
    } else {
        for (Dword i = 0; i < at<std::uint16_t>(resource, 14); ++i) {
            if (!enabled(i))
                continue;
            auto current = reinterpret_cast<const Dword *>(at<float *>(owner, 24)) + i * 4;
            auto destination = reinterpret_cast<Dword *>(output) + indices[i];
            destination[0] = current[0];
            destination[1] = current[1];
            destination[2] = current[2];
        }
    }
    if (at<Byte>(resource, 17))
        constant_vectors(resource, output);
    return 1;
}
} // namespace

void allocate_vector_block_buffers(void *owner) {
    auto resource = at<void *>(owner, 12);
    Dword count = at<std::uint16_t>(resource, 14);
    if (!count)
        return;
    at<void *>(owner, 24) = at<void *>(owner, 20) = allocate_animation_buffer(count << 4);
    at<void *>(owner, 36) = at<void *>(owner, 32) = allocate_animation_buffer(count << 4);
    at<void *>(owner, 44) = allocate_animation_buffer(count * 48);
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    for (Dword i = 0; i < at<std::uint16_t>(resource, 14); ++i) {
        auto encoded = offset(resource, 20 + i * 36);
        auto coefficients = at<float *>(owner, 44) + i * 12;
        for (unsigned axis = 0; axis < 3; ++axis) {
            coefficients[axis] = at<float>(encoded, axis * 4);
            float range = at<float>(encoded, 12 + axis * 4);
            coefficients[axis + 3] = range;
            coefficients[axis + 3] = float(X(coefficients[axis + 3]) * 0.00001525902189314365386962890625f);
            auto value = Extended::integer(at<std::uint16_t>(encoded, 24 + axis * 2), format) *
                         range * 0.00001525902189314365386962890625f;
            coefficients[axis + 6] = float(value + value - range);
            value = Extended::integer(at<std::uint16_t>(encoded, 30 + axis * 2), format) * range *
                    0.00001525902189314365386962890625f;
            coefficients[axis + 9] = float(value + value);
            coefficients[axis + 9] = float(X(coefficients[axis + 9]) * 0.0039215688593685626983642578125f);
        }
    }
}

Dword sample_masked_vector_blocks(void *owner, float time, float *output, const Dword *mask) {
    return sample_vector_blocks(owner, time, output, mask);
}

Dword __fastcall sample_block_vector_animation(void *owner, void *, Dword time, Dword output,
                                              Dword mask) {
    if (!at<void *>(owner, 24))
        allocate_vector_block_buffers(owner);
    return mask ? sample_masked_vector_blocks(owner, channel_float(time),
                                              reinterpret_cast<float *>(output),
                                              reinterpret_cast<const Dword *>(mask))
                : sample_vector_blocks(owner, channel_float(time),
                                       reinterpret_cast<float *>(output), nullptr);
}
} // namespace nfs::game::world::actor_resources::animation_runtime
