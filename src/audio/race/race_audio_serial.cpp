#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::race_audio {
// Same saved producer, separated from resource binding clients.
Dword next_emitter_serial() {
    return ++pending::emitter_serial;
}
} // namespace nfs::game::runtime::race_audio
