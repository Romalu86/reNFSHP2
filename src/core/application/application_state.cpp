#include "nfs/core/application/application.hpp"
namespace nfs::application {

// Startup state and shared exception-text storage.
alignas(4) Byte startup_storage[4369]{};
char (&executable_path)[260] = *reinterpret_cast<char (*)[260]>(startup_storage);
Byte &os_major_above_four = startup_storage[260];
Byte &windows_2000 = startup_storage[261];
Byte &xp_or_later = startup_storage[262];
HANDLE &running_mutex = *reinterpret_cast<HANDLE *>(startup_storage + 4364);
Byte &shutdown_reached = startup_storage[4368];
FILE *null_output{};
kernel::ErrorHandler &previous_error_handler =
    *reinterpret_cast<kernel::ErrorHandler *>(startup_storage + 4360);
GraphicsFailure graphics_failure{};
Byte cursor_shown = 1;  
Dword movie_counter{};
namespace pending {
char *const exception_text = reinterpret_cast<char *>(startup_storage + 264);
}
namespace messages {
#include "application_messages.inc"
}
} // namespace nfs::application
namespace nfs::network {
HANDLE &instance_mutex =
    *reinterpret_cast<HANDLE *>(application::startup_storage + 4364);  
} // namespace nfs::network
