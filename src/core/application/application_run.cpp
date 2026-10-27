#include "nfs/core/application/application.hpp"
#include "nfs/core/errors/assertions.hpp"
#include "nfs/core/console/console.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/graphics/display/graphics_error.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/resources/localization/region.hpp"
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <intrin.h>
namespace nfs::application {
namespace {
// Authorized compatibility: a C++ failure after successful initialization still
// releases the initialized engine. Raw SEH faults keep the existing WinMain path.
void shutdown_after_run_failure() noexcept {
    try {
        lifecycle::shutdown();
    } catch (...) {
        // Preserve the original run failure if shutdown also throws.
    }
}
} // namespace
// is a continuation in this body, not another function.
int run(int count, const char *const *arguments) {
    launch::parse(&launch::options, count, arguments);
    if (!launch::field<Dword>(8627752) && !running_mutex) {
        running_mutex = OpenMutexA(2031617, FALSE, "hp2_exe_RUNNING_MUTEX");
        bool another_instance = running_mutex != nullptr;
        if (!running_mutex) {
            running_mutex = CreateMutexA(nullptr, FALSE, "hp2_exe_RUNNING_MUTEX");
            another_instance = GetLastError() == 183;
        }
        if (another_instance) {
            MessageBoxA(nullptr, "Need For Speed is already running.",
                        "Need For Speed Hot Pursuit 2", 48);
            return 1;
        }
    }
    lifecycle::memory_bootstrap();
    lifecycle::initialize();
    const int startup_error = lifecycle::startup_error;
    if (startup_error) {
        try {
            lifecycle::shutdown();
        } catch (...) {
            // Restore the captured error code after the catch.
        }
        const wchar_t *text = messages::corrupt_files[region::get()];
        switch (startup_error) {
        case 1:
            text = graphics::unresolved::error_message;
            break;
        case 2:
            text = messages::graphics_mode[region::get()];
            break;
        case 3:
            text = messages::not_enough_space[region::get()];
            break;
        case 4:
            text = messages::graphics_hardware[region::get()];
            break;
        case 5:
            text = messages::low_disk_space[region::get()];
            break;
        case 8:
            text = messages::unsupported_device[region::get()];
            break;
        }
        MessageBoxW(nullptr, text, messages::error_caption[region::get()], 48);
        return 1;
    }
    std::srand(static_cast<unsigned>(__rdtsc()));
    try {
        run_outer_flow();
    } catch (...) {
        shutdown_after_run_failure();
        throw;
    }
    shutdown_reached = 1;
    lifecycle::shutdown();
    kernel::print::initialize();
    pending::write_memory_report("memory");
    pending::release_controller_resources();
    return 0;
}

// Preserve both SetErrorMode calls, including the reset to zero.
int probe_error_mode() {
    SetErrorMode(2);
    SetErrorMode(0);
    return 0;
}


void __cdecl handle_error(const char *format, ...) {
    char message[512];
    std::va_list arguments;
    va_start(arguments, format);
    if (format)
        _vsnprintf(message, 511, format, arguments);
    else
        message[0] = 0;
    va_end(arguments);
    if (assertions::report(message) && previous_error_handler)
        previous_error_handler(message);
}

// Complete saved XOR EAX,EAX; RET callback.
Dword __cdecl ignore_graphics_failure() {
    return 0;
}

// Complete RET callback; arguments belong to PRINT API.
void __cdecl release_console_write(int, const char *) {
}


void set_graphics_failure(GraphicsFailure callback) {
    graphics_failure = callback;
}

// The saved success path does not close its registry handle.
int read_user_region() {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Control Panel\\International\\", 0, 131097, &key) == 0) {
        BYTE locale[500]{};
        DWORD type = 1, bytes = 500;
        if (RegQueryValueExA(key, "Locale", nullptr, &type, locale, &bytes) == 0) {
            const Dword language =
                std::strtoul(reinterpret_cast<const char *>(locale), nullptr, 16) & 1023;
            switch (language) {
            case 7:
                return 2;
            case 10:
                return 4;
            case 12:
                return 1;
            case 16:
                return 3;
            case 18:
                return 6;
            case 29:
                return 5;
            }
        }
    }
    return 0;
}
} // namespace nfs::application
