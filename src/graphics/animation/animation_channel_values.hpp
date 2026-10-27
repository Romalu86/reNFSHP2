#pragma once
#include "runtime_animation_dispatch.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world::actor_resources::animation_runtime {
using kernel::numeric::Extended;
inline float channel_float(Dword word) {
    float value;
    std::memcpy(&value, &word, 4);
    return value;
}
inline Dword channel_word(float value) {
    Dword word;
    std::memcpy(&word, &value, 4);
    return word;
}
inline int channel_integer(const Extended &value) {
    auto integer = value.round_to_integer();
    return integer < INT32_MIN || integer > INT32_MAX ? INT32_MIN : int(integer);
}
inline int channel_frame(float time) {
    auto format = kernel::numeric::current_format();
    Extended value(time, format);
    float adjusted = float(time < 0 || std::isnan(time) ? value + 0.499999f : value - 0.499999f);
    return channel_integer(Extended(adjusted, format));
}
inline int channel_frame(const Extended &time) {
    auto format = kernel::numeric::current_format();
    bool negative = time < Extended(0, format) || std::isnan(float(time));
    float adjusted = float(negative ? time + 0.499999f : time - 0.499999f);
    return channel_integer(Extended(adjusted, format));
}
int delta_frame_from_time(float);
void mirror_animation_pose(void *, void *, void *, Byte, const Dword *);
void decode_delta_values(void *, Dword, Dword, int, int, const float *, float *);
void interpolate_delta_values(void *, Dword, int, int, int, const float *, float *, int,
                              const std::uint16_t *, float);
void decode_channel_record(void *, const Byte *, Byte *);
} // namespace nfs::game::world::actor_resources::animation_runtime
