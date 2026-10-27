#pragma once
#include "nfs/core/types/kernel.hpp"

namespace nfs::kernel::cpu {
struct Registers {
    Dword eax, ebx, edx, ecx;
};
struct Information {
    std::uint64_t frequency;
    char model_name[16];
    Dword processors;
    char vendor[16];
    Dword family, model, stepping, pentium_pro, intel, amd, cyrix, centaur;
    Dword timestamp_counter, conditional_move, streaming, mmx, sse, sse2, amd_3d;
};
static_assert(sizeof(Information) == 104);
extern Dword processors, family, model, stepping, pentium_pro, intel, amd, cyrix, centaur;
extern Dword timestamp_counter, conditional_move, sse2, amd_3d;
extern char vendor[16];
extern std::uint8_t initialized;
extern Dword frequency;
extern std::uint64_t frequency_wide;
extern Information information;
Dword has_cpuid();
void identify(Registers *, Dword);
Dword legacy_cyrix();
Dword registry_megahertz();
Dword measure_clock();
const char *model_name();
Dword initialize();
} // namespace nfs::kernel::cpu
