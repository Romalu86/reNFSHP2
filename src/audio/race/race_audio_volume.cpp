#include "nfs/physics/frame/simulation_runtime.hpp"
namespace nfs::game::runtime::race_audio {
// IMUL keeps the low 32 bits before the signed shift.
int group_volume(Dword input, Dword group, Dword multiplier) {
    unsigned offset;
    switch (group) {
    case 1:
        offset = 2352;
        break;
    case 2:
    case 6:
    case 7:
    case 8:
        offset = 2336;
        break;
    case 3:
        offset = 2356;
        break;
    case 4:
        offset = 2328;
        break;
    case 5:
        offset = 2332;
        break;
    default:
        return 0;
    }
    return static_cast<int>(at<Dword>(system(72), offset) * input * multiplier) >> 14;
}
} // namespace nfs::game::runtime::race_audio
