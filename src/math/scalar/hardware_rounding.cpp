#include "../precision/frame_x87_bridge.hpp"

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "This conversion requires the native x87 format");

namespace nfs::game::runtime::hardware {
// Both leaves use the active hardware rounding mode and integer-indefinite result.
int current_integer_f32(float value) {
    return __builtin_lrintl(static_cast<long double>(value));
}

long long current_integer_native80(const void *bytes) {
    long double value;
    __builtin_memcpy(&value, bytes, 10);
    return __builtin_llrintl(value);
}
} // namespace nfs::game::runtime::hardware
