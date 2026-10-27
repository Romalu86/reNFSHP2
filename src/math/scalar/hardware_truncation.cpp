#include "../precision/frame_x87_bridge.hpp"

#pragma STDC FENV_ACCESS ON

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "This conversion requires the native x87 format");
static_assert(sizeof(long long) == 8, "The conversion result is a signed QWORD");

namespace nfs::game::runtime::hardware {
long long current_integer_native80(const void *bytes);

long long truncate_integer_native80(const void *bytes) {
    const int previous_rounding = __builtin_flt_rounds();
    __builtin_set_flt_rounds(0);
    const long long result = current_integer_native80(bytes);
    __builtin_set_flt_rounds(previous_rounding);
    return result;
}
} // namespace nfs::game::runtime::hardware
