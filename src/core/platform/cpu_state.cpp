#include "nfs/core/platform/cpu.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::kernel::cpu {

Dword processors = 0, family = 0, model = 0, stepping = 0, pentium_pro = 0;
Dword intel = 0, amd = 0, cyrix = 0, centaur = 0;
Dword timestamp_counter = 0, conditional_move = 0, sse2 = 0, amd_3d = 0;
char vendor[16] = {};
std::uint8_t initialized = 0;
Dword frequency = 0;
std::uint64_t frequency_wide = 0;
Information information = {};
static const char *selected_model = "Pentium";

// Keep the mutable fallback when no vendor flag is set.
const char *model_name() {
    if (intel) {
        selected_model = "Pentium";
        if (pentium_pro) {
            selected_model = "PentiumPro";
            if (cpu_mmx) {
                selected_model = "Pentium2";
                if (cpu_sse)
                    selected_model = "Pentium3";
            }
        }
        if (sse2)
            selected_model = "Pentium4";
    } else if (amd) {
        selected_model = "K6";
        if (amd_3d) {
            selected_model = "K6-2";
            if (pentium_pro) {
                selected_model = "Athlon";
                if (cpu_sse)
                    selected_model = processors > 1 ? "AthlonMP" : "AthlonXP";
            }
        }
    } else if (cyrix)
        selected_model = "Cyrix";
    else if (centaur)
        selected_model = "WinChip";
    return selected_model;
}

} // namespace nfs::kernel::cpu
