#include "retained_transcendentals.hpp"

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "The retained argument loader requires the x87 format");

namespace nfs::kernel::numeric::hardware {

__declspec(noinline) long double read_retained_argument(const void *argument) {
    return *static_cast<const long double *>(argument);
}

} // namespace nfs::kernel::numeric::hardware
