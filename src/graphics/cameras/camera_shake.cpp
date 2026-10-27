#include "camera_director_runtime.hpp"
namespace nfs::game::world::camera {
using kernel::numeric::Extended;
namespace {
Extended X(float value) {
    return Extended(value, kernel::numeric::current_format());
}
Extended sine(float value) {
    return Extended::from_double(double(runtime::turn_sine(value)),
                                 kernel::numeric::current_format());
}
void apply_shake(float *position, const float *axes, float x, float y, Extended z) {
    const float weighted_y = float((X(axes[1]) + 1.f) * y);
    const float weighted_z = float((X(axes[2]) + 1.f) * z);
    position[0] = float((X(axes[0]) + 1.f) * x + position[0]);
    position[1] = float(X(weighted_y) + position[1]);
    position[2] = float(X(weighted_z) + position[2]);
}
Extended shake_component(float a, float b, float c, float primary, float secondary,
                         float tertiary) {
    const float tail = float(sine(c) * tertiary);
    const float middle = float(sine(b) * secondary + tail);
    return sine(a) * primary + middle;
}
} // namespace

void add_road_shake(float *position, float magnitude, const float *axes) {
    if (magnitude <= 0)
        return;
    if (12.f < magnitude)
        magnitude = 12.f;
    const float primary = float(X(magnitude) * 0.01f);
    const float secondary = float(X(magnitude) * 0.005f);
    const float tertiary = float(X(magnitude) * 0.0045f);
    const auto tick =
        Extended::integer(static_cast<std::int32_t>(graphics::unresolved::camera_state),
                          kernel::numeric::current_format());
    const float a = float(X(magnitude) * tick * 0.03022284992039203643798828125f);
    const float b = float(tick * 0.3314867913722991943359375f);
    const auto c = tick * 0.02148677967488765716552734375f;
    const float retained_c = float(c);
    const float x =
        float(shake_component(float(X(a) * 0.123571097850799560546875f), float(X(b) * 0.123571097850799560546875f),
                              float(c * 0.123571097850799560546875f), primary, secondary, tertiary));
    const float y = float(shake_component(a, b, retained_c, primary, secondary, tertiary));
    const auto z =
        shake_component(float(X(a) * 0.117532096803188323974609375f), float(X(b) * 0.117532096803188323974609375f),
                        float(X(retained_c) * 0.117532096803188323974609375f), primary, secondary, tertiary);
    apply_shake(position, axes, x, y, z);
}

void add_impact_shake(float *position, float magnitude, const float *axes) {
    if (magnitude <= 0)
        return;
    const float primary = float(X(magnitude) * 0.01f);
    const float secondary = float(X(magnitude) * 0.012f);
    const float tertiary = float(X(magnitude) * 0.011f);
    const auto tick =
        Extended::integer(static_cast<std::int32_t>(graphics::unresolved::camera_state),
                          kernel::numeric::current_format());
    const float a = float(tick * 1.05f), b = float(tick * 0.9f);
    const auto c = tick * 0.75f;
    const float retained_c = float(c);
    const float x = float(shake_component(float(X(a) - 1.834f), float(X(b) - 0.569f),
                                          float(c + 0.369f), primary, secondary, tertiary));
    const float y =
        float(shake_component(float(X(a) + 1.298f), float(X(b) + 2.345f),
                              float(X(retained_c) + 3.591f), primary, secondary, tertiary));
    const auto z = shake_component(float(X(a) + 4.623f), float(X(b) + 5.195f),
                                   float(X(retained_c) + 6.398f), primary, secondary, tertiary);
    apply_shake(position, axes, x, y, z);
}
} // namespace nfs::game::world::camera
