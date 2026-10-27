#include "../rendering/vehicle_renderable_frame.hpp"
namespace nfs::game::runtime::physics {
using kernel::numeric::Extended;
using world::at;
using world::offset;
struct PanelTuning {
    float spring, damping, response;
};
PanelTuning panel_tuning[2] = {{2.4f, .2f, 100.f}, {2.4f, .3f, 200.f}};
static void integrate_panel(void *car, float limit, unsigned angle_offset,
                            const PanelTuning &tuning, bool hood) {
    auto *angles = reinterpret_cast<float *>(offset(car, angle_offset));
    auto *velocities = angles + 4;
    if (limit <= 0) {
        angles[0] = angles[1] = angles[3] = angles[2] = 0;
        return;
    }
    const Dword elapsed =
        graphics::unresolved::camera_state - graphics::previous_frame_tick;
    if (!elapsed)
        return;
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    const auto step = Extended::integer(int(elapsed), format) * 0.01666666753590106964111328125f;
    auto acceleration = X(at<float>(at<void *>(car, 1768), 112)) * .25f;
    if (!(acceleration < X(0)))
        acceleration = X(0);
    const auto angle_middle = X(angles[2]) + angles[1],
               velocity_middle = X(velocities[2]) + velocities[1];
    auto force =
        (-acceleration - (angle_middle + angle_middle + angles[3] + angles[0]) * 0.16666667163372039794921875f) *
            tuning.spring -
        (velocity_middle + velocity_middle + velocities[3] + velocities[0]) * tuning.damping *
            0.16666667163372039794921875f;
    float magnitude = at<float>(car, 3156);
    if (magnitude < 0)
        magnitude = -magnitude;
    if (magnitude > .1f)
        force = hood ? X(at<float>(car, 3156)) / X(tuning.response) + force
                     : force - X(at<float>(car, 3156)) / X(tuning.response);
    magnitude = at<float>(car, 3164);
    if (magnitude < 0)
        magnitude = -magnitude;
    if (magnitude > .35f)
        force = hood ? force - X(at<float>(car, 3164)) / X(tuning.response)
                     : force - (X(3) / X(tuning.response)) * at<float>(car, 3164);
    magnitude = at<float>(car, 3196);
    if (magnitude < 0)
        magnitude = -magnitude;
    if (magnitude > .01f)
        force = hood ? force - X(at<float>(car, 3196)) * 12.f
                     : X(at<float>(car, 3196)) * 36.f + force;
    if (hood && at<float>(car, 3168) > 0)
        force = (X(at<float>(car, 3168)) * .3f) / X(tuning.response) + force;
    velocities[0] = velocities[1];
    const float middle_velocity = velocities[2];
    velocities[2] = velocities[3];
    velocities[1] = middle_velocity;
    velocities[3] = float(force * step + velocities[3]);
    const auto updated_middle = X(velocities[2]) + velocities[1];
    angles[0] = angles[1];
    angles[1] = angles[2];
    angles[2] = angles[3];
    angles[3] = float((updated_middle + updated_middle + velocities[3] + velocities[0]) * step *
                          0.16666667163372039794921875f +
                      angles[3]);
    const auto updated_angles = X(angles[2]) + angles[1];
    const auto average = (updated_angles + updated_angles + angles[3] + angles[0]) * 0.16666667163372039794921875f;
    if (average < X(0)) {
        const auto sum = X(velocities[2]) + velocities[1];
        const float reflected =
            float((sum + sum + velocities[3] + velocities[0]) * -0.0333333350718021392822265625f);
        for (unsigned i = 0; i < 4; ++i)
            velocities[i] = reflected;
        angles[0] = angles[1] = angles[3] = angles[2] = 0;
        return;
    }
    const auto last_middle = X(angles[2]) + angles[1];
    if (X(limit) < (last_middle + last_middle + angles[3] + angles[0]) * 0.16666667163372039794921875f) {
        const auto sum = X(velocities[2]) + velocities[1];
        const float reflected =
            float((sum + sum + velocities[3] + velocities[0]) * -0.083333335816860198974609375f);
        for (unsigned i = 0; i < 4; ++i)
            velocities[i] = reflected;
        angles[0] = angles[1] = angles[3] = angles[2] = limit;
    }
}

void animate_trunk(void *car, float limit) {
    integrate_panel(car, limit, 1628, panel_tuning[0], false);
}

void animate_hood(void *car, float limit) {
    integrate_panel(car, limit, 1660, panel_tuning[1], true);
}
} // namespace nfs::game::runtime::physics
