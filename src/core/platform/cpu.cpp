#include "nfs/core/platform/cpu.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <intrin.h>
#include <cstdlib>
#include <cstring>

namespace nfs::kernel::cpu {
// The byte-DIV hardware probe is compiled separately with checked code generation.


Dword registry_megahertz() {
    Dword result = 0;
    DWORD type = REG_DWORD, bytes = 4;
    HKEY key;
    if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                    &key) == 0) {
        RegQueryValueExA(key, "~MHz", nullptr, &type, reinterpret_cast<BYTE *>(&result), &bytes);
        RegCloseKey(key);
    }
    return result;
}

// Sample five intervals and retain the second-lowest result from the four samples after warm-up.
 
 
Dword measure_clock() {
    LARGE_INTEGER ticks_per_second;
    QueryPerformanceFrequency(&ticks_per_second);
    Dword lowest = 0, second_lowest = 0;
    for (int sample = 0; sample < 5; ++sample) {
        LARGE_INTEGER start, current;
        start.LowPart = 0;
        QueryPerformanceCounter(&start);
        std::uint64_t remaining = 1000000;
        do {
            current.LowPart = 0;
            QueryPerformanceCounter(&current);
            if (current.QuadPart == start.QuadPart && --remaining == 0)
                break;
        } while (std::uint64_t(current.QuadPart - start.QuadPart) == 0);
        std::uint64_t begin = __rdtsc();
        remaining = 1000000;
        start.LowPart = 0;
        QueryPerformanceCounter(&start);
        do {
            current.LowPart = 0;
            QueryPerformanceCounter(&current);
            if (current.QuadPart == start.QuadPart && --remaining == 0)
                break;
        } while (std::uint64_t(current.QuadPart - start.QuadPart) <
                 std::uint64_t(ticks_per_second.QuadPart) / 5);
        Dword elapsed = static_cast<Dword>(__rdtsc() - begin);
        if (sample) {
            if (elapsed < lowest || !lowest) {
                second_lowest = lowest;
                lowest = elapsed;
            } else if (elapsed < second_lowest || !second_lowest)
                second_lowest = elapsed;
        }
    }
    return second_lowest ? second_lowest : lowest;
}

// This also produces the existing memory dispatch flags.
Dword initialize() {
    std::uint64_t clock = frequency_wide;
    if (!initialized) {
        sse2 = cpu_sse = amd_3d = cpu_mmx = processors = conditional_move = 0;
        stepping = model = family = cpu_stream_mmx = timestamp_counter = pentium_pro = 0;
        centaur = amd = cyrix = intel = 0;
        vendor[0] = 0;
        if (!has_cpuid())
            cyrix = legacy_cyrix();
        else {
            SYSTEM_INFO system_info;
            GetSystemInfo(&system_info);
            processors = system_info.dwNumberOfProcessors;
            Registers registers;
            identify(&registers, 0);
            Dword maximum_leaf = registers.eax;
            std::memcpy(vendor, &registers.ebx, 12);
            vendor[12] = 0;
            if (static_cast<int>(maximum_leaf) > 0) {
                intel = std::strcmp(vendor, "GenuineIntel") == 0;
                if (!intel) {
                    amd = std::strcmp(vendor, "AuthenticAMD") == 0;
                    if (!amd) {
                        cyrix = std::strcmp(vendor, "CyrixInstead") == 0;
                        if (!cyrix)
                            centaur = std::strcmp(vendor, "CentaurHauls") == 0;
                    }
                }
                identify(&registers, 1);
                family = registers.eax >> 8 & 15;
                model = registers.eax >> 4 & 15;
                stepping = registers.eax & 15;
                if (family > 5)
                    pentium_pro = 1;
                identify(&registers, 2147483648u);
                if (amd && registers.eax > 2147483648u) {
                    identify(&registers, 2147483649u);
                    amd_3d = registers.edx >> 31;
                    cpu_stream_mmx = registers.edx >> 22 & 1;
                }
                identify(&registers, 1);
                sse2 = registers.edx >> 26 & 1;
                cpu_mmx = registers.edx >> 23 & 1;
                cpu_sse = registers.edx >> 25 & 1;
                timestamp_counter = registers.edx >> 4 & 1;
                conditional_move = registers.edx >> 15 & 1;
                if (cpu_sse)
                    cpu_stream_mmx = 1;
                if (const char *setting = std::getenv("ISMMX"))
                    cpu_mmx = std::atoi(setting);
                if (const char *setting = std::getenv("ISK3D"))
                    amd_3d = std::atoi(setting);
                OSVERSIONINFOA version;
                version.dwOSVersionInfoSize = sizeof(version);
                GetVersionExA(&version);
                if (version.dwPlatformId == 1 && version.dwMinorVersion == 0)
                    sse2 = cpu_sse = 0;
                if (const char *setting = std::getenv("ISSSE"))
                    cpu_sse = std::atoi(setting);
                if (const char *setting = std::getenv("ISSSE2"))
                    sse2 = std::atoi(setting);
                if (const char *setting = std::getenv("ISMOVNT"))
                    cpu_stream_mmx = std::atoi(setting);
                if (static_cast<int>(maximum_leaf) > 1)
                    identify(&registers, 2);
            }
        }
        Dword measured = registry_megahertz(), multiplier = 1000000;
        if (!measured) {
            measured = measure_clock();
            multiplier = 5;
        }
        clock = std::uint64_t(measured) * multiplier;
        frequency = static_cast<Dword>(clock);
        if (clock >> 32 || frequency == 4294967295u)
            frequency = 4294967295u;
        initialized = 1;
    }
    frequency_wide = clock;
    if (!frequency) {
        memory_zero(&information, sizeof(information));
        return frequency;
    }
    information.frequency = clock;
    std::strcpy(information.model_name, model_name());
    information.processors = processors;
    std::strcpy(information.vendor, vendor);
    information.family = family;
    information.model = model;
    information.stepping = stepping;
    information.pentium_pro = pentium_pro;
    information.intel = intel;
    information.amd = amd;
    information.cyrix = cyrix;
    information.centaur = centaur;
    information.timestamp_counter = timestamp_counter;
    information.conditional_move = conditional_move;
    information.streaming = cpu_stream_mmx;
    information.mmx = cpu_mmx;
    information.sse = cpu_sse;
    information.sse2 = sse2;
    information.amd_3d = amd_3d;
    return frequency;
}
} // namespace nfs::kernel::cpu
