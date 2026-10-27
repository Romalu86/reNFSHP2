#include "d3dx_matrix_dispatch.hpp"
#include <windows.h>
#include <intrin.h>
#include <cstring>
namespace nfs::graphics::d3dx {
Dword dispatch_state = 65535;

Dword read_dispatch_registry(DWORD expected_type, const char *name, BYTE *output, DWORD bytes) {
    HKEY key = nullptr;
    if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Direct3D", &key) != ERROR_SUCCESS)
        return 0;
    DWORD type;
    LSTATUS result = RegQueryValueExA(key, name, nullptr, &type, output, &bytes);
    RegCloseKey(key);
    return result == ERROR_SUCCESS && type == expected_type;
}
// CPUID remains guarded against the saved illegal-instruction path.
Dword processor_has_packed_math() {
    Dword present = 0;
    __try {
        int registers[4];
        __cpuid(registers, int(2147483648u));
        if (Dword(registers[0]) > 2147483648u) {
            __cpuid(registers, int(2147483649u));
            present = Dword(registers[3]) >> 31;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return present;
}
// Continue within the same exception frame.
Dword processor_streaming_features() {
    Dword features = 0;
    __try {
        int registers[4];
        __cpuid(registers, 0);
        if (registers[0]) {
            __cpuid(registers, 1);
            if (Dword(registers[3]) & 33554432u)
                features |= 4;
            if (Dword(registers[3]) & 67108864u)
                features |= 8;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return features;
}

Dword dispatch_processor_feature(DWORD feature) {
    OSVERSIONINFOA version{};
    version.dwOSVersionInfoSize = 148;
    BOOL read = GetVersionExA(&version);
    bool unavailable = !read;
    if (version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
        if (version.dwMajorVersion < 5 &&
            (version.dwMajorVersion != 4 || version.dwMinorVersion < 10 ||
             (version.dwBuildNumber & 65535) < 1373))
            unavailable = true;
    } else {
        if (version.dwPlatformId != VER_PLATFORM_WIN32_NT)
            return 0;
        if (read) {
            if (feature != 10)
                return IsProcessorFeaturePresent(feature);
            return processor_streaming_features() & 8;
        }
    }
    if (feature == 6) {
        if (!unavailable)
            return processor_streaming_features() & 4;
    } else if (feature == 7)
        return processor_has_packed_math();
    else if (feature == 10 && !unavailable)
        return processor_streaming_features() & 8;
    return 0;
}

int pending::initialize_dispatch(int request) {
    if (!request) {
        dispatch_state = 65535;
        for (unsigned index = 0; index < 57; ++index)
            dispatch[index] = default_dispatch[index];
    } else if (dispatch_state == 65535) {
        dispatch_state = 0;
        for (unsigned index = 0; index < 57; ++index)
            dispatch[index] = default_dispatch[index];
        install_baseline(dispatch);
        if (!read_dispatch_registry(REG_DWORD, "DisableD3DXPSGP",
                                    reinterpret_cast<BYTE *>(&request), 4))
            request = 0;
        if (request != 1) {
            if (dispatch_processor_feature(7)) {
                install_packed(dispatch);
                dispatch_state = 1;
            } else if (dispatch_processor_feature(10)) {
                install_sse2(dispatch);
                dispatch_state = 2;
            } else if (dispatch_processor_feature(6)) {
                install_sse(dispatch);
                dispatch_state = 3;
            }
        }
    }
    return int(dispatch_state);
}
} // namespace nfs::graphics::d3dx
