#include "nfs/network/session/network_runtime.hpp"
namespace nfs::network {

bool has_instance_mutex() {
    return instance_mutex != nullptr;
}
 
 
bool acquire_instance_mutex() {
    if (instance_mutex)
        return true;
    instance_mutex = OpenMutexA(2031617, FALSE, "hp2.exe_RUNNING_MUTEX");
    if (instance_mutex)
        return false;
    instance_mutex = CreateMutexA(nullptr, FALSE, "hp2.exe_RUNNING_MUTEX");
    return GetLastError() != 183;
}
} // namespace nfs::network
