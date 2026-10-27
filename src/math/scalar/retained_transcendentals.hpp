#pragma once

namespace nfs::kernel::numeric::hardware {

// The argument points to the retained provider's 80-bit local.  The public
// boundary carries only an address; each numeric result returns in ST0.
__declspec(noinline) long double read_retained_argument(const void *argument);
long double sine_retained(const void *argument);
long double tangent_retained(const void *argument);
long double arctangent_retained(const void *argument);
long double unit_remainder_retained(const void *argument);

} // namespace nfs::kernel::numeric::hardware
