#include "d3dx_dispatch_kernels.hpp"
#include <intrin.h>
#include <cstring>
namespace nfs::graphics::d3dx {
// This helper has no SEH fallback in the function.
Dword packed_processor_capabilities() {
    int registers[4];
    __cpuid(registers, 0);
    __cpuid(registers, 0);
    Dword features = 1;
    char vendor[13];
    std::memcpy(vendor, &registers[1], 4);
    std::memcpy(vendor + 4, &registers[3], 4);
    std::memcpy(vendor + 8, &registers[2], 4);
    vendor[12] = 0;
    if (!registers[0])
        return features;
    features |= 2;
    __cpuid(registers, 1);
    if (Dword(registers[3]) & 8388608u)
        features |= 32;
    __cpuid(registers, int(2147483648u));
    if (Dword(registers[0]) <= 2147483648u)
        return features;
    features |= 4;
    __cpuid(registers, int(2147483649u));
    if (Dword(registers[3]) & 2147483648u)
        features |= 128;
    if (std::memcmp(vendor, "AuthenticAMD", 12) == 0) {
        if (Dword(registers[3]) & 1073741824u)
            features |= 256;
        if (Dword(registers[3]) & 4194304u)
            features |= 512;
    }
    return features;
}
} // namespace nfs::graphics::d3dx
