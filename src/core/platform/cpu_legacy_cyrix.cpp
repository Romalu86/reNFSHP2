#include "nfs/core/platform/cpu.hpp"
#include <intrin.h>

#if !defined(__INTELLISENSE__) && (!defined(__clang__) || !defined(_M_IX86))
#error This hardware probe requires the checked Clang Win32 code generator.
#endif

namespace nfs::kernel::cpu {

// A separate call keeps the operands live across flag preparation without ASM.
__declspec(noinline) void prepare_cyrix_division_flags() {
    const Dword flags = __readeflags();
    __writeeflags(flags & ~2261u);
}


// The build checks DIV BL and the complete flag-transport instruction sequence.
__declspec(noinline) Dword legacy_cyrix() {
    volatile unsigned char dividend = 5;
    volatile unsigned char divisor = 2;
    const unsigned char denominator = divisor;
    const unsigned char numerator = dividend;
    prepare_cyrix_division_flags();
    [[maybe_unused]] volatile unsigned char quotient =
        static_cast<unsigned char>(numerator / denominator);
    const unsigned char status = static_cast<unsigned char>(__readeflags());
    return status == 2;
}
} // namespace nfs::kernel::cpu
