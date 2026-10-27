#include "nfs/core/application/application.hpp"
#include "nfs/core/errors/assertions.hpp"
#include "nfs/core/console/console.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/resources/localization/region.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/input/window/window_input.hpp"
#include <cctype>
#include <cstdlib>
#include <cstring>

// The entry returns with RET16: all four WinMain arguments.
int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR command_line, int) {
    using namespace nfs;
    using namespace nfs::application;
    services::pending::application_instance = instance;
    region::set(read_user_region());
    // Clear all 39 DWORDs before requesting the extended layout.
    OSVERSIONINFOEXA version{};
    static_assert(sizeof(version) == 156);
    version.dwOSVersionInfoSize = sizeof(version);
    if (GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&version))) {
        if (version.dwMajorVersion > 4)
            os_major_above_four = 1;
        if (version.dwMajorVersion == 5) {
            if (!version.dwMinorVersion)
                windows_2000 = 1;
            if (version.dwMinorVersion > 0)
                xp_or_later = 1;
        }
        if (version.dwMajorVersion > 5)
            xp_or_later = 1;
    }
    HMODULE direct3d = LoadLibraryA("D3D8.DLL");
    if (!direct3d) {
        const wchar_t *caption = messages::error_caption[region::get()];
        MessageBoxW(nullptr, messages::directx_required[region::get()], caption, 48);
        return 1;
    }
    FreeLibrary(direct3d);

    char *command = static_cast<char *>(std::malloc(std::strlen(command_line) + 1));
    std::strcpy(command, command_line);
    GetModuleFileNameA(nullptr, executable_path, 259);
    executable_path[259] = 0;
    auto **arguments = static_cast<char **>(std::malloc(404));

    kernel::memory_zero(arguments, 101);
    arguments[0] = executable_path;
    arguments[1] = command;
    int count = 2;
    char *cursor = command;
    while (*cursor) {
        if (!std::isspace(static_cast<unsigned char>(*cursor)))
            ++cursor;
        else {
            *cursor++ = 0;
            if (count >= 100)
                break;
            while (std::isspace(static_cast<unsigned char>(*cursor)))
                ++cursor;
            if (!*cursor)
                break;
            arguments[count++] = cursor;
        }
    }
    // UCRT fopen skips standard streams. Reopen the just-closed stdout FILE explicitly; freopen
    // preserves _SH_DENYNO sharing.
     
    std::fclose(stdout);
    null_output = std::freopen("NUL", "wt", stdout);
    previous_error_handler = kernel::error_handler;
    kernel::error_handler = handle_error;
    assertions::set_abort_only(static_cast<Byte>(probe_error_mode()));
    set_graphics_failure(ignore_graphics_failure);
    kernel::print::initialize();
    kernel::print::register_device(1, "console", release_console_write);
    window_input::set_system_shortcuts(0);

    __try {
        run(count, arguments);
    } __except (pending::exception_filter(GetExceptionInformation())) {
        pending::recover_display();
        pending::recover_keyboard_hook();
        if (!shutdown_reached)
            MessageBoxA(nullptr, pending::exception_text,
                        "Need For Speed Hot Pursuit 2 (exception error)", 0);
    }
    window_input::restore_system_shortcuts();
    std::free(command);
    std::free(arguments);
    std::fflush(null_output);
    std::fclose(null_output);
    return 0;
}
