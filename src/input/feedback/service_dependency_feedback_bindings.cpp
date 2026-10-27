#include "nfs/core/services/service_dependencies.hpp"
namespace nfs::services::pending {
Byte enable_feedback(Dword device) {
    return dependency::enable_feedback(device);
}
void set_feedback_gain(Dword device, Dword gain) {
    dependency::set_feedback_gain(device, gain);
}
Dword create_feedback_effect(Dword device, Dword *definition) {
    return dependency::create_feedback_effect(device, definition);
}
} // namespace nfs::services::pending
