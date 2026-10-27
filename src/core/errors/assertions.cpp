#include "nfs/core/errors/assertions.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/core/console/console.hpp"
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include <cstdio>
#include <cstdlib>
// This exact CRT API is imported from the system MSVCRT. The old statically
// linked CRT's complete UI/termination contract remains a compatibility boundary.
extern "C" __declspec(dllimport) void __cdecl _amsg_exit(int);
namespace nfs::assertions {
namespace {
void diagnostic(const char *file, Dword number, const char *expression, const char *information) {
    char message[512];
    _snprintf(message, sizeof(message), "Assertion Failed: %s(%d) : %s : %s\n", file, number,
              expression, information);
    kernel::print::write(2, message);
}
Dword show(const char *file, Dword number, const char *expression, const char *information) {
    diagnostic(file, number, expression, information);
    const char *action = abort_only ? "Abort?" : "(Press Retry to debug the application)";
    char text[512];
    _snprintf(text, 511, "Assert:\t%s\nFile:\t%s\nLine:\t%d\nInfo:\t%s\n%s", expression, file,
              number, information, action);
    const int answer =
        MessageBoxA(nullptr, text, "SSAssert - Assertion Failed", (abort_only ? 2 : 0) + 73746);
    if (answer == 3 || answer == 6)
        std::exit(0);
    if (answer == 4 || answer == 7)
        kernel::debug_break();
    return 0;
}
} // namespace

void prepare() {
    if (abort_only)
        kernel::debug_break();
}

void set_abort_only(unsigned char value) {
    abort_only = value;
}

Dword report_details(const char *file, Dword number, const char *expression,
                    const char *information) {
    return show(file, number, expression, information);
}
// Callers only consume the AL=0 result.
unsigned char report(const char *format, ...) {
    std::va_list arguments;
    va_start(arguments, format);
    const auto result = report_v(format, arguments);
    va_end(arguments);
    return result;
}
unsigned char report_v(const char *format, std::va_list arguments) {
    char information[1024];
    _vsnprintf(information, 1023, format, arguments);
    information[1023] = 0;
    return static_cast<unsigned char>(show(source, line, description, information));
}

void pure_virtual_call() {
    _amsg_exit(25);
}
} // namespace nfs::assertions
namespace nfs::game::runtime::physics::pending {
void prepare_error() {
    assertions::prepare();
}
void report_runtime_error(const char *format, ...) {
    std::va_list arguments;
    va_start(arguments, format);
    assertions::report_v(format, arguments);
    va_end(arguments);
}
} // namespace nfs::game::runtime::physics::pending
