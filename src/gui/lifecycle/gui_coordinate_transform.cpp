#include "nfs/gui/engine/gui_engine.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstdint>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "GUI coordinates retain the native x87 intermediate");

namespace nfs::gui::engine {
namespace {
int scaled_coordinate(int value, float scale, float offset) {
    const volatile float stored = static_cast<float>(
        static_cast<long double>(value) * static_cast<long double>(scale) +
        static_cast<long double>(offset));
    return game::runtime::hardware::current_integer_f32(stored);
}
int sum_coordinates(int first, int second) {
    return static_cast<std::int32_t>(std::uint32_t(first) + std::uint32_t(second));
}
int coordinate_difference(int first, int second) {
    return static_cast<std::int32_t>(std::uint32_t(first) - std::uint32_t(second));
}
}

// Each signed sum is retained through scale/offset,
// stored to F32, then converted by the caller's x87 rounding mode.
Rectangle *to_pixels(Rectangle *out, const Rectangle *source) {
    const int bottom = scaled_coordinate(sum_coordinates(source->height, source->top),
                                         vertical_scale, vertical_offset);
    const int right = scaled_coordinate(sum_coordinates(source->width, source->left),
                                        horizontal_scale, horizontal_offset);
    const int top = scaled_coordinate(source->top, vertical_scale, vertical_offset);
    const int left = scaled_coordinate(source->left, horizontal_scale, horizontal_offset);
    // Loads/conversions complete before these ordered stores, including when the two rectangles
    // occupy overlapping storage.
    volatile Rectangle *output = out;
    output->left = left;
    output->width = coordinate_difference(right, left);
    output->methods = rectangle_methods;
    output->top = top;
    output->height = coordinate_difference(bottom, top);
    return out;
}
}
