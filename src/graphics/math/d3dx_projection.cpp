#include "d3dx_dispatch_kernels.hpp"
#include <cstring>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Projection arithmetic requires the retained x87 compiler.");

namespace nfs::graphics::d3dx {
namespace {
long double viewport_integer(const volatile Dword *word) {
    const Dword bits = *word;
    std::int32_t signed_value;
    std::memcpy(&signed_value, &bits, sizeof(signed_value));
    long double value = static_cast<long double>(signed_value);
    // The unsigned conversion is a signed DWORD load followed by this add.
    // A direct unsigned conversion would omit its precision-control rounding.
    if (signed_value < 0)
        value += 4294967296.0f;
    return value;
}

float viewport_depth(const volatile Dword *word) {
    const Dword bits = *word;
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

const float *combine_projection(float *temporary, const float *projection, const float *view,
                                const float *world) {
    switch ((world ? 4 : 0) | (view ? 2 : 0) | (projection ? 1 : 0)) {
    case 0: {
        volatile float *identity = temporary;
        identity[14] = 0.0f;
        identity[13] = 0.0f;
        identity[12] = 0.0f;
        identity[11] = 0.0f;
        identity[9] = 0.0f;
        identity[8] = 0.0f;
        identity[7] = 0.0f;
        identity[6] = 0.0f;
        identity[4] = 0.0f;
        identity[3] = 0.0f;
        identity[2] = 0.0f;
        identity[1] = 0.0f;
        identity[15] = 1.0f;
        identity[10] = 1.0f;
        identity[5] = 1.0f;
        identity[0] = 1.0f;
        return temporary;
    }
    case 1:
        return projection;
    case 2:
        return view;
    case 3:
        multiply_bootstrap(temporary, view, projection);
        break;
    case 4:
        return world;
    case 5:
        multiply_bootstrap(temporary, world, projection);
        break;
    case 6:
        multiply_bootstrap(temporary, world, view);
        break;
    case 7:
        multiply_bootstrap(temporary, world, view);
        multiply_bootstrap(temporary, temporary, projection);
        break;
    }
    return temporary;
}
} // namespace

float *__stdcall project_point3_scalar(float *out, const float *point, const Dword *viewport,
                                       const float *projection, const float *view,
                                       const float *world) {
    alignas(16) float temporary[16];
    const float *combined = combine_projection(temporary, projection, view, world);
    transform_coordinate3_bootstrap(out, point, combined);
    if (viewport) {
        const volatile Dword *port = viewport;
        volatile float *coordinate = out;
        long double horizontal = static_cast<long double>(coordinate[0]) + 1.0f;
        const long double width = viewport_integer(port + 2);
        horizontal *= width;
        horizontal *= .5f;
        horizontal += viewport_integer(port);
        coordinate[0] = static_cast<float>(horizontal);

        long double vertical = 1.0L - coordinate[1];
        const long double height = viewport_integer(port + 3);
        vertical *= height;
        vertical *= .5f;
        vertical += viewport_integer(port + 1);
        coordinate[1] = static_cast<float>(vertical);

        long double depth = static_cast<long double>(viewport_depth(port + 5));
        depth -= viewport_depth(port + 4);
        depth *= coordinate[2];
        depth += viewport_depth(port + 4);
        coordinate[2] = static_cast<float>(depth);
    }
    return out;
}

float *__stdcall unproject_point3_scalar(float *out, const float *point, const Dword *viewport,
                                         const float *projection, const float *view,
                                         const float *world) {
    alignas(16) float inverse[16];
    const float *combined = combine_projection(inverse, projection, view, world);
    if (projection || view || world)
        invert_matrix_bootstrap(inverse, nullptr, combined);
    if (viewport) {
        const volatile Dword *port = viewport;
        const volatile float *source = point;
        volatile float *coordinate = out;
        const long double left = viewport_integer(port);
        long double horizontal = static_cast<long double>(source[0]) - left;
        horizontal /= viewport_integer(port + 2);
        horizontal += horizontal;
        horizontal -= 1.0f;
        coordinate[0] = static_cast<float>(horizontal);

        const long double top = viewport_integer(port + 1);
        long double vertical = static_cast<long double>(source[1]) - top;
        vertical /= viewport_integer(port + 3);
        vertical += vertical;
        vertical -= 1.0f;
        coordinate[1] = static_cast<float>(-vertical);

        long double depth = static_cast<long double>(source[2]);
        depth -= viewport_depth(port + 4);
        long double depth_range = static_cast<long double>(viewport_depth(port + 5));
        depth_range -= viewport_depth(port + 4);
        depth /= depth_range;
        coordinate[2] = static_cast<float>(depth);
        point = out;
    }
    transform_coordinate3_bootstrap(out, point, inverse);
    return out;
}
} // namespace nfs::graphics::d3dx
