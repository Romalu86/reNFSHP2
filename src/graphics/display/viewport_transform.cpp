#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include <intrin.h>
#include <cstring>

namespace nfs::graphics::viewports {
namespace {
__declspec(noinline) Dword raw_argument_bits(const float *argument) {
    Dword bits;
    std::memcpy(&bits, argument, sizeof(bits));
    return bits;
}
}

namespace numeric_x87 {
void perspective_prefix(Dword *, Dword, Dword, Dword, Dword);
void perspective_tail(Dword *, float, float);
}

// The MSVC wrapper forwards the original argument DWORDs before their raw
// copies are saved by the prefix. Only the arithmetic tail is retained.
void perspective(Dword *object, float degrees, float aspect, float near_plane, float far_plane) {
    const Dword degree_bits = raw_argument_bits(&degrees);
    const Dword aspect_bits = raw_argument_bits(&aspect);
    const Dword near_bits = raw_argument_bits(&near_plane);
    const Dword far_bits = raw_argument_bits(&far_plane);
    numeric_x87::perspective_prefix(object, degree_bits, aspect_bits, near_bits, far_bits);
    numeric_x87::perspective_tail(object, degrees, aspect);
}


void set_matrix(Dword *object, const float *matrix) {
    __movsd(reinterpret_cast<unsigned long *>(object + 116 / 4),
            reinterpret_cast<const unsigned long *>(matrix), 16);
    if (reinterpret_cast<const std::uint8_t *>(object)[360])
        refresh(object + 3);
}


void refresh(Dword *object) {
    end(reinterpret_cast<Dword *>(object[356 / 4]));
    begin(reinterpret_cast<Dword *>(object[356 / 4]));
}

} // namespace nfs::graphics::viewports
