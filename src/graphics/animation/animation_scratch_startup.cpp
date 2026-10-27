#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include <cstdint>
namespace nfs::lifecycle::runtime {


alignas(16) static std::uint8_t animation_storage[16384]{};
 
void initialize_animation_storage() {
    const Dword aligned = (reinterpret_cast<Dword>(animation_storage) + 15u) & 4294967280u;
    animation_buffer = reinterpret_cast<void *>(aligned);
}
} // namespace nfs::lifecycle::runtime
NFS_KERNEL_INITIALIZER(startup_initialize_animation_storage, ".CRT$XCU_NFS_006902D8",
                       nfs::lifecycle::runtime::initialize_animation_storage);
