#include "animation_channel_values.hpp"
#include <utility>
namespace nfs::game::world::actor_resources::animation_runtime {
extern Byte reset_backward_animation_blocks;
namespace {
bool rotation_enabled(const Dword *mask, Byte bone) {
    return !mask || (mask[bone >> 5] & (Dword(1) << (bone & 31)));
}
void decode_rotation(float *out, const Byte *encoded) {
    auto format = kernel::numeric::current_format();
    auto words = reinterpret_cast<const std::uint16_t *>(encoded);
    for (unsigned i = 0; i < 3; ++i)
        out[i] = float(Extended::integer(words[i] >> 4, format) * 0.0004884005174972116947174072265625f - 1.0f);
    unsigned fourth = ((encoded[0] & 15) * 16 + (encoded[2] & 15)) * 16 + (encoded[4] & 15);
    out[3] = float(Extended::integer(fourth, format) * 0.0004884005174972116947174072265625f - 1.0f);
}
void apply_rotation_delta(void *owner, unsigned channel, const Byte *encoded, const float *source,
                          float *out, bool forward) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    auto coefficients = at<const float *>(owner, 16) + channel * 8;
    unsigned values[4] = {
        unsigned(encoded[0] >> 2), unsigned(encoded[1] >> 2), unsigned(encoded[2] >> 2),
        unsigned(((encoded[0] & 3) * 4 + (encoded[1] & 3)) * 4 + (encoded[2] & 3))};
    auto x =
        Extended::integer(values[0], format) * coefficients[4] * 0.01587301678955554962158203125f + coefficients[0];
    auto y =
        Extended::integer(values[1], format) * coefficients[5] * 0.01587301678955554962158203125f + coefficients[1];
    float z = float(Extended::integer(values[2], format) * coefficients[6] * 0.01587301678955554962158203125f +
                    coefficients[2]);
    float w = float(Extended::integer(values[3], format) * coefficients[7] * 0.01587301678955554962158203125f +
                    coefficients[3]);
    out[0] = float(forward ? x + source[0] : X(source[0]) - x);
    out[1] = float(forward ? y + source[1] : X(source[1]) - y);
    out[2] = float(forward ? X(z) + source[2] : X(source[2]) - z);
    out[3] = float(forward ? X(w) + source[3] : X(source[3]) - w);
}
void advance_rotation_deltas(void *owner, Byte *block, void *resource, int first, int last,
                             float *cache, const Dword *mask) {
    auto indices = at<const Byte *>(resource, 12);
    bool forward = first < last;
    auto encoded = block + (Dword(first) + 2) * at<Byte>(resource, 6) * 3 - (forward ? 0 : 3);
    while (first != last) {
        if (forward) {
            for (unsigned channel = 0; channel < at<Byte>(resource, 6); ++channel, encoded += 3)
                if (rotation_enabled(mask, indices[channel]))
                    apply_rotation_delta(owner, channel, encoded, cache + channel * 4,
                                         cache + channel * 4, true);
            ++first;
        } else {
            for (int channel = int(at<Byte>(resource, 6)) - 1; channel >= 0;
                 --channel, encoded -= 3)
                if (rotation_enabled(mask, indices[channel]))
                    apply_rotation_delta(owner, unsigned(channel), encoded, cache + channel * 4,
                                         cache + channel * 4, false);
            --first;
        }
    }
}
void prepare_following_rotation(void *owner, void *resource, int next, int current_block,
                                Dword local_frame, const Dword *mask) {
    if (next == at<int>(owner, 40))
        return;
    unsigned shift = at<Byte>(resource, 16) & 31;
    int next_block = next >> shift;
    auto block = at<Byte *>(owner, 20) + Dword(next_block) * at<Dword>(owner, 24);
    auto indices = at<const Byte *>(resource, 12);
    for (unsigned channel = 0; channel < at<Byte>(resource, 6); ++channel) {
        if (!rotation_enabled(mask, indices[channel]))
            continue;
        auto out = at<float *>(owner, 48) + channel * 4;
        if (next_block == current_block) {
            auto encoded = block + (local_frame + 2) * at<Byte>(resource, 6) * 3 + channel * 3;
            apply_rotation_delta(owner, channel, encoded, at<float *>(owner, 36) + channel * 4, out,
                                 true);
        } else
            decode_rotation(out, block + channel * 6);
    }
    at<int>(owner, 40) = next;
}
Dword sample_rotation_blocks(void *owner, float time, void *pose, const Dword *mask) {
    if (at<Dword>(owner, 60) != Dword(mask)) {
        at<Dword>(owner, 60) = Dword(mask);
        at<int>(owner, 28) = -1;
        at<int>(owner, 40) = -1;
    }
    auto resource = at<void *>(owner, 12);
    auto indices = at<const Byte *>(resource, 12);
    if (at<Byte>(resource, 6)) {
        int rounded = channel_frame(time), frame;
        auto keys = at<const std::uint16_t *>(resource, 8);
        int frames = at<std::uint16_t>(resource, 4);
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
        if (at<int>(owner, 40) == frame) {
            std::swap(at<float *>(owner, 48), at<float *>(owner, 36));
            at<int>(owner, 40) = cached;
        } else {
            if (Dword(cached) == Dword(frame) + 1) {
                for (unsigned i = 0; i < at<Byte>(resource, 6) * 4u; ++i)
                    at<Dword *>(owner, 48)[i] = at<Dword *>(owner, 36)[i];
                cached = at<int>(owner, 28);
                at<int>(owner, 40) = cached;
            }
            auto block = at<Byte *>(owner, 20) + at<Dword>(owner, 24) * Dword(block_index);
            bool reset_backward = cached > frame && reset_backward_animation_blocks == 0;
            Dword cursor;
            if (cached == -1 || (cached >> shift) != block_index || (!mask && !local_frame) ||
                reset_backward) {
                for (unsigned channel = 0; channel < at<Byte>(resource, 6); ++channel)
                    if (rotation_enabled(mask, indices[channel]))
                        decode_rotation(at<float *>(owner, 36) + channel * 4, block + channel * 6);
                cursor = 0;
            } else
                cursor = Dword(cached) & low_mask;
            advance_rotation_deltas(owner, block, resource, int(cursor), int(local_frame),
                                    at<float *>(owner, 36), mask);
        }
        at<int>(owner, 28) = frame;
        auto format = kernel::numeric::current_format();
        auto X = [&](float value) { return Extended(value, format); };
        float start = !keys ? float(rounded) : frame ? float(keys[frame - 1]) : 0.0f;
        bool interpolate = !std::isnan(time) && time != start;
        float fraction = time;
        if (interpolate) {
            if (!keys)
                fraction = float(X(time) - start);
            else if (!frame)
                fraction = float(X(time) / Extended::integer(keys[0], format));
            else
                fraction =
                    float((X(time) - start) / (Extended::integer(keys[frame], format) - start));
        }
        interpolate = interpolate && frame < frames - 1;
        if (interpolate)
            prepare_following_rotation(owner, resource, frame + 1, block_index, local_frame, mask);
        for (unsigned channel = 0; channel < at<Byte>(resource, 6); ++channel) {
            if (!rotation_enabled(mask, indices[channel]))
                continue;
            auto out = reinterpret_cast<Dword *>(offset(pose, Dword(indices[channel]) * 48 + 16));
            auto current = at<Dword *>(owner, 36) + channel * 4;
            auto following = at<float *>(owner, 48) + channel * 4;
            for (unsigned component = 0; component < 4; ++component)
                out[component] = interpolate
                                     ? channel_word(float((X(following[component]) -
                                                           channel_float(current[component])) *
                                                              fraction +
                                                          channel_float(current[component])))
                                     : current[component];
        }
    }
    auto constants = at<const Byte *>(owner, 52);
    for (unsigned channel = 0; channel < at<Byte>(resource, 7); ++channel)
        if (rotation_enabled(mask, constants[channel]))
            decode_rotation(
                reinterpret_cast<float *>(offset(pose, Dword(constants[channel]) * 48 + 16)),
                at<const Byte *>(owner, 56) + channel * 6);
    return 1;
}
} // namespace

void __fastcall prepare_compact_rotation_frame(void *owner, void *, void *resource, int next,
                                               int block, Dword local_frame) {
    prepare_following_rotation(owner, resource, next, block, local_frame, nullptr);
}

void __fastcall advance_masked_rotation_deltas(void *owner, void *, Byte *block, void *resource,
                                               int first, int last, float *cache,
                                               const Dword *mask) {
    if (first < last)
        advance_rotation_deltas(owner, block, resource, first, last, cache, mask);
}

void __fastcall rewind_masked_rotation_deltas(void *owner, void *, Byte *block, void *resource,
                                              int first, int last, float *cache, const Dword *mask) {
    if (last <= first - 1)
        advance_rotation_deltas(owner, block, resource, first, last, cache, mask);
}

void __fastcall prepare_masked_rotation_frame(void *owner, void *, void *resource, int next,
                                              int block, Dword local_frame, const Dword *mask) {
    prepare_following_rotation(owner, resource, next, block, local_frame, mask);
}

Dword __fastcall sample_masked_compact_rotation_pose(void *owner, void *, Dword time, Dword pose,
                                                    Dword mask) {
    return sample_rotation_blocks(owner, channel_float(time), reinterpret_cast<void *>(pose),
                                  reinterpret_cast<const Dword *>(mask));
}

Dword __fastcall sample_compact_rotation_pose(void *owner, void *, Dword time, Dword pose, Dword mask) {
    if (mask)
        return sample_masked_compact_rotation_pose(owner, nullptr, time, pose, mask);
    return sample_rotation_blocks(owner, channel_float(time), reinterpret_cast<void *>(pose),
                                  nullptr);
}
} // namespace nfs::game::world::actor_resources::animation_runtime
