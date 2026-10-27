#pragma once
#include "nfs/core/types/kernel.hpp"

namespace nfs::game::runtime {
// This private return is consumed only by retained-precision particle TUs.
long double retained_emitter_period(kernel::Dword *);
// Keep the public MSVC long-double wrapper on its original binary64 boundary.
double compatibility_emitter_period(kernel::Dword *);
}
