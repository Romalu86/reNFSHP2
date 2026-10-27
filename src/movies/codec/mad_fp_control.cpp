#include "mad_codec.hpp"
#include <float.h>
namespace nfs::movies::mad::codec {
// Abstract control-word merge, using the compiler CRT for native transport.
Dword change_fp_control(Dword value, Dword mask) {
    constexpr Dword legacy_fields = 983839;
    Dword control = _control87(0, 0) & legacy_fields;
    Dword merged = (control & ~mask) | (value & mask);
    Dword native = merged;
    // Map reserved abstract precision mode 3 to native 24-bit precision.
    if ((native & 196608) == 196608)
        native = (native & ~196608) | 131072;
    _control87(native, legacy_fields);
    return merged;
}
} // namespace nfs::movies::mad::codec
