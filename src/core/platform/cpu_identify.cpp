#include "nfs/core/platform/cpu.hpp"
#include <intrin.h>
namespace nfs::kernel::cpu {
// Restore EFLAGS after probing the writable ID bit.
Dword has_cpuid() {
    Dword before = __readeflags();
    __writeeflags(before ^ 2097152);
    Dword changed = __readeflags() ^ before;
    __writeeflags(before);
    return changed;
}

// Preserve EAX/EBX/EDX/ECX storage order.
void identify(Registers *output, Dword leaf) {
    int registers[4];
    __cpuid(registers, static_cast<int>(leaf));
    output->eax = registers[0];
    output->ebx = registers[1];
    output->edx = registers[3];
    output->ecx = registers[2];
}

} // namespace nfs::kernel::cpu
