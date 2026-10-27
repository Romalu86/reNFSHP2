#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstring>
namespace nfs::game::runtime::particles::pending {

void initialize_emitter(Dword *emitter, const void *record) {
    std::strncpy(reinterpret_cast<char *>(emitter), reinterpret_cast<const char *>(record), 32);
    at<Byte>(emitter, 31) = 0;
    std::strncpy(reinterpret_cast<char *>(bytes(emitter) + 233),
                 reinterpret_cast<const char *>(bytes(record) + 84), 12);
    at<Byte>(emitter, 244) = 0;
    at<Dword>(emitter, 252) = race_audio::next_emitter_serial();
    at<int>(emitter, 248) =
        race_audio::find_emitter_sound(reinterpret_cast<char *>(bytes(emitter) + 233));
    for (unsigned i = 0; i < 3; ++i)
        at<Dword>(emitter, 44 + i * 4) = at<Dword>(record, 32 + i * 4);
    at<void *>(emitter, 32) = bytes(emitter) + 44;
    at<const float *>(emitter, 36) = saved::zero_vector;
}
} // namespace nfs::game::runtime::particles::pending
