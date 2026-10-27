#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::particles {
// Fields omitted by the initializer retain their bytes.
void reset_type(Dword *object) {
    at<Dword>(object, 196) = 0;
    at<float>(object, 28) = at<float>(object, 32) = 1;
    at<Dword>(object, 36) = 0;
    at<Dword>(object, 0) = 0;
    at<float>(object, 4) = at<float>(object, 8) = 1;
    at<Dword>(object, 12) = 0;
    at<float>(object, 16) = at<float>(object, 20) = 1;
    at<Dword>(object, 24) = 0;
    for (unsigned offset = 100; offset <= 128; offset += 4)
        at<Dword>(object, offset) = 0;
    at<float>(object, 132) = at<float>(object, 136) = 1;
    at<Dword>(object, 140) = 0;
    at<float>(object, 148) = 1;
    at<Dword>(object, 144) = 0;
    for (unsigned offset = 152; offset <= 168; offset += 4)
        at<Dword>(object, offset) = 0;
    at<float>(object, 212) = 1;
    for (unsigned offset = 200; offset <= 208; offset += 4)
        at<Dword>(object, offset) = 0;
    at<float>(object, 172) = -0.5f;
    at<float>(object, 176) = 0.5f;
    at<float>(object, 180) = 1;
    at<float>(object, 184) = -1;
    at<float>(object, 188) = 1;
    at<float>(object, 192) = 2;
    for (unsigned offset = 40; offset <= 88; offset += 12) {
        at<float>(object, offset) = at<float>(object, offset + 4) = 1;
        at<Dword>(object, offset + 8) = 0;
    }
    for (unsigned offset = 216; offset <= 260; offset += 4)
        at<float>(object, offset) = 255;
}
} // namespace nfs::game::runtime::particles
