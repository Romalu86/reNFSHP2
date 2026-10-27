#include "nfs/core/services/service_dependencies.hpp"
namespace nfs::services::dependency::pending {
const GUID effect_guids[13] = {
#include "service_dependency_effect_guids.inc"
};
} // namespace nfs::services::dependency::pending
