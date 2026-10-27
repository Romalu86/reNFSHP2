#include "nfs/gui/lifecycle/gui_coordinates.hpp"
#include "gui_render_numeric_x87.hpp"
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::graphics::gui_coordinates {

float *screen_position(float *out, float x, float y, float origin_x, float origin_y, float width,
                       float height) {

    auto E = [](float value) { return gui_numeric::number(value); };
    const auto normalized_x = (E(x) - (E(width) * 0.5f + origin_x)) / E(width);
    out[0] = float(normalized_x + normalized_x);
    const auto normalized_y = (E(y) - (E(height) * 0.5f + origin_y)) / E(height);
    out[1] = float(-(normalized_y + normalized_y));
    out[2] = -1;
    out[3] = 1;
    return out;
}

float *vector(float *out, float x, float y, float z, float w) {
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
    return out;
}
} // namespace nfs::graphics::gui_coordinates
