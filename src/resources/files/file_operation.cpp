#include "nfs/resources/files/file_operation.hpp"
namespace nfs {
std::uint32_t file_operation_key(const FileOperation32& request) noexcept {
    return ((request.token>>5)&16777215u)|(std::uint32_t(request.priority)<<24);
}
}
