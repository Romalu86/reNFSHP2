#include "nfs/core/types/kernel.hpp"
namespace nfs::kernel {

Dword signal_ready(Signal *signal) {
    return WaitForSingleObject(signal->native, 0) == WAIT_OBJECT_0 ? 1u : 0u;
}
} // namespace nfs::kernel
