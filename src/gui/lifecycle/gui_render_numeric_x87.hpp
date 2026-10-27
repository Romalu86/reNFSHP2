#pragma once
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstdint>
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "GUI numeric paths require retained x87 intermediates");
namespace nfs::gui::retained {
template <class T> inline T read_field(const T &value) {
    return *static_cast<const volatile T *>(&value);
}
template <class T> inline void write_field(T &destination, T value) {
    *static_cast<volatile T *>(&destination) = value;
}
inline long double number(float value) {
    return static_cast<long double>(value);
}
inline long double integer(std::int64_t value) {
    return static_cast<long double>(value);
}
inline std::uint32_t truncate(long double value) {
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
}
inline int round_f32(float value) {
    return game::runtime::hardware::current_integer_f32(value);
}
} // namespace nfs::gui::retained
