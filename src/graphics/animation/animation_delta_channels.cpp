#include "animation_channel_values.hpp"
#include <cmath>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Delta animation requires native x87 intermediates");
namespace nfs::game::world::actor_resources::animation_runtime {

void retain_delta_frame(void *owner, int frame) {
    void *resource = at<void *>(owner, 12);
    int count = at<std::uint16_t>(resource, 8);
    if (frame >= count)
        frame = count - 1;
    else if (frame < 0)
        frame = 0;
    auto *data = at<void *>(resource, 4);
    auto *buffer = at<float *>(owner, 20);
    decode_delta_values(data, 0, at<std::uint16_t>(data, 0), at<int>(owner, 16), frame, buffer,
                        buffer);
    at<int>(owner, 16) = frame;
}

void retain_sparse_delta_frame(void *owner, int frame) {
    void *data = at<void *>(at<void *>(owner, 12), 4);
    auto *buffer = at<float *>(owner, 20);
    decode_delta_values(data, 0, at<std::uint16_t>(data, 0), at<int>(owner, 16), frame, buffer,
                        buffer);
    at<int>(owner, 16) = frame;
}

int sparse_delta_frame(void *owner, float time) {
    int frame = delta_frame_from_time(time);
    void *resource = at<void *>(owner, 12);
    auto *keys = at<std::uint16_t *>(resource, 8);
    if (time < float(*keys))
        return 0;
    int index = at<int>(owner, 16) < 1 ? 0 : int(Dword(at<int>(owner, 16)) - 1);
    if (frame < int(keys[index])) {
        while (index > 0 && frame < int(keys[index]))
            --index;
    } else {
        int last = int(at<std::uint16_t>(resource, 12)) - 2;
        while (index < last && int(keys[index + 1]) <= frame)
            ++index;
    }
    return index + 1;
}
namespace {
void sample_fixed_delta(void *owner, float time, Dword output, int width) {
    int frame = delta_frame_from_time(time);
    retain_delta_frame(owner, frame);
    void *resource = at<void *>(owner, 12), *data = at<void *>(resource, 4);
    Dword count = at<std::uint16_t>(data, 0);
    auto *mapping = reinterpret_cast<std::uint16_t *>(offset(resource, 10));
    const volatile float stored_frame = float(frame);
    if (time != stored_frame && int(Dword(frame) + 1) < int(at<std::uint16_t>(resource, 8))) {
        const volatile float fraction = float(static_cast<long double>(time) - stored_frame);
        interpolate_delta_values(data, 0, int(count), frame, int(Dword(frame) + 1),
                                 at<float *>(owner, 20),
                                 reinterpret_cast<float *>(output), width, mapping, fraction);
    } else if (width == 1) {
        for (Dword index = 0; index < count; ++index) {
            const Dword *source = at<Dword *volatile>(owner, 20);
            const Dword destination = mapping[index];
            reinterpret_cast<Dword *>(output)[destination] = source[index];
        }
    } else {
        for (Dword component = 0, group = 0; component < count; component += 4, ++group) {
            const Dword *source = at<Dword *volatile>(owner, 20) + component;
            Dword *destination = reinterpret_cast<Dword *>(output) + mapping[group];
            for (unsigned axis = 0; axis < 4; ++axis)
                at<volatile Dword>(destination, axis * 4) = at<volatile Dword>(source, axis * 4);
        }
    }
}
bool sparse_fraction(const std::uint16_t *keys, int key_count, int frame, float time,
                     volatile float &fraction) {
    Dword previous = frame ? keys[frame - 1] : 0;
    float start = float(previous);
    if (time == start)
        return false;
    if (frame == key_count - 1 && time > float(keys[key_count - 2]))
        return false;
    if (!frame && time < 0)
        return false;
    const Dword interval_bits = Dword(keys[frame]) - previous;
    const auto interval = static_cast<long double>(std::int32_t(interval_bits));
    fraction = float((static_cast<long double>(time) - start) / interval);
    return true;
}
} // namespace

void __fastcall advance_delta_scalar_animation(void *receiver, void *, Dword, Dword second_time,
                                               Dword output) {
    sample_fixed_delta(receiver, channel_float(second_time), output, 1);
}
// The third word is ignored in the RET12 method.
Dword __fastcall sample_delta_scalar_animation(void *receiver, void *, Dword time, Dword output,
                                              Dword) {
    sample_fixed_delta(receiver, channel_float(time), output, 1);
    return 1;
}

void __fastcall advance_delta_quaternion_animation(void *receiver, void *, Dword, Dword second_time,
                                                   Dword output) {
    sample_fixed_delta(receiver, channel_float(second_time), output, 4);
}

Dword __fastcall sample_delta_quaternion_animation(void *receiver, void *, Dword time, Dword output,
                                                  Dword) {
    sample_fixed_delta(receiver, channel_float(time), output, 4);
    return 1;
}

Dword __fastcall sample_sparse_delta_scalars(void *receiver, void *, Dword time_word, Dword output,
                                            Dword) {
    float time = channel_float(time_word);
    int frame = sparse_delta_frame(receiver, time);
    retain_sparse_delta_frame(receiver, frame);
    void *resource = at<void *>(receiver, 12), *data = at<void *>(resource, 4);
    Dword count = at<std::uint16_t>(data, 0);
    volatile float fraction;
    auto *mapping = reinterpret_cast<std::uint16_t *>(offset(resource, 14));
    const auto *keys = at<const std::uint16_t *>(resource, 8);
    const int key_count = at<std::uint16_t>(resource, 12);
    if (sparse_fraction(keys, key_count, frame, time, fraction))
        interpolate_delta_values(data, 0, int(count), frame, int(Dword(frame) + 1),
                                 at<float *>(receiver, 20), reinterpret_cast<float *>(output), 1,
                                 mapping, fraction);
    else
        for (Dword index = 0; index < count; ++index) {
            const Dword *source = at<Dword *volatile>(receiver, 20);
            const Dword destination = mapping[index];
            reinterpret_cast<Dword *>(output)[destination] = source[index];
        }
    return 1;
}

Dword __fastcall sample_sparse_delta_quaternions(void *receiver, void *, Dword time_word, Dword output,
                                                Dword) {
    float time = channel_float(time_word);
    void *resource = at<void *>(receiver, 12);
    void *data = at<void *>(resource, 4);
    const int key_count = at<std::uint16_t>(resource, 12);
    const auto *keys = at<const std::uint16_t *>(resource, 8);
    Dword total = at<std::uint16_t>(data, 0);
    auto *mapping = reinterpret_cast<std::uint16_t *>(offset(resource, 14));
    int frame = sparse_delta_frame(receiver, time);
    retain_sparse_delta_frame(receiver, frame);
    volatile float fraction;
    auto X = [](float value) { return static_cast<long double>(value); };
    if (sparse_fraction(keys, key_count, frame, time, fraction)) {
        for (Dword component = 0, group = 0; component < total; component += 4, ++group) {
            float *first = at<float *>(receiver, 20) + component, second[4];
            decode_delta_values(data, component, 4, frame, int(Dword(frame) + 1),
                                first, second);
            first = at<float *>(receiver, 20) + component;
            auto dot = X(second[3]) * first[3] + X(second[2]) * first[2] + X(second[0]) * first[0] +
                       X(second[1]) * first[1];
            bool reverse = !(X(0) < dot);
            float *destination = reinterpret_cast<float *>(output) + mapping[group];
            for (unsigned axis = 0; axis < 4; ++axis)
                destination[axis] =
                    float(reverse ? X(first[axis]) - (X(second[axis]) + first[axis]) * fraction
                                  : (X(second[axis]) - first[axis]) * fraction + first[axis]);
            auto square = X(destination[0]) * destination[0] + X(destination[1]) * destination[1] +
                          X(destination[2]) * destination[2] + X(destination[3]) * destination[3];
            auto inverse = X(1) / __builtin_sqrtl(square);

            for (unsigned axis = 0; axis < 4; ++axis)
                destination[axis] = float(inverse * destination[axis]);
        }
    } else {
        for (Dword component = 0, group = 0; component < total; component += 4, ++group) {
            const Dword *source = at<Dword *volatile>(receiver, 20) + component;
            Dword *destination = reinterpret_cast<Dword *>(output) + mapping[group];
            for (unsigned axis = 0; axis < 4; ++axis)
                at<volatile Dword>(destination, axis * 4) = at<volatile Dword>(source, axis * 4);
        }
    }
    return (total & 4294967040u) | 1;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
