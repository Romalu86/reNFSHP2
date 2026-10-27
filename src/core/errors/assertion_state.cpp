#include "nfs/core/errors/assertions.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::assertions {
const char empty_text[1] = {0}; // Both assertion pointers share this empty string.
const char *source = empty_text, *description = empty_text;
Dword line{};
unsigned char abort_only = 1;
} // namespace nfs::assertions
namespace nfs::game::runtime::physics::pending {
const char *&error_source = assertions::source;
Dword &physics_assertion_line = assertions::line;
} // namespace nfs::game::runtime::physics::pending
