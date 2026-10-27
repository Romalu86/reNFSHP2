#include "race_render_passes.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <initializer_list>
namespace nfs::game::world::fog {
using kernel::Dword;
using kernel::numeric::Extended;
// Scalar state is scattered in the FogSystem layout.
static Extended interpolate(float *origin, float *current, float target, float *difference,
                             float *increment, int *frame, int frames) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    const float frame_count = float(Extended::integer(frames, format));
    if (frames < 1)
        return E(*current);
    if (*current == target) {
        *frame = 1;
        *difference = *increment = 0;
        return E(*current);
    }
    if (*frame == int(Dword(frames) - 1)) {
        *current = target;
        return E(target);
    }
    const float difference_squared = float(E(*difference) * *difference);
    const float difference_magnitude = float(E(difference_squared).square_root());
    if (difference_magnitude < 0.000000000009999999960041972002500187954865396022796630859375f) {
        *origin = *current;
        const auto distance = E(target) - *current;
        *difference = float(distance);
        *increment = float(distance / E(frame_count));
    }
    float phase = 0;
    if (*difference != 0) {
        const auto position = Extended::integer(*frame, format) * *increment + *origin;
        phase = float(E(1) - (E(target) - position) / E(*difference) - 0.5f);
    }
    const float phase_squared = float(E(phase) * phase);
    const float phase_magnitude = float(E(phase_squared).square_root());
    float weight = 0;
    if (phase_magnitude > 0) {
        const float radians = float(E(phase) * 3.1415927410125732421875f);
        const float sine = float(Extended::from_double(std::sin(double(radians)), format));
        weight = float((E(sine) + 1.f) * 0.5f);
    }
    const auto offset = E(weight) * *difference;
    const float offset_squared = float(offset * offset);
    const float offset_magnitude = float(E(offset_squared).square_root());
    if (offset_magnitude > 0) {
        *frame = int(Dword(*frame) + 1);
        *current = float(E(weight) * *difference + *origin);
        return E(*current);
    }
    if (*frame == frames / 2)
        *current = float(E(*difference) * 0.5f + *origin);
    const float result = *current;
    *frame = int(Dword(*frame) + 1);
    return E(result);
}

void configure(State *fog, float density, float scale, float start, float finish,
                 Dword vertex_mode, Dword colour, int frames) {
    fog->vertex_mode = vertex_mode;
    fog->alpha = colour >> 24;
    fog->blue = colour & 255;
    fog->green = colour >> 8 & 255;
    fog->frames = frames;
    fog->red = colour >> 16 & 255;
    for (Transition *transition : {&fog->start_transition, &fog->end_transition,
                                   &fog->density_transition, &fog->scale_transition})
        if (!transition->frame)
            transition->frame = 1;
    auto change = [frames](Transition &transition, float &current, float target) {
        interpolate(&transition.origin, &current, target, &transition.difference,
                     &transition.increment, &transition.frame, frames);
    };
    change(fog->start_transition, fog->start, start);
    change(fog->end_transition, fog->finish, finish);
    change(fog->density_transition, fog->density, density);
    change(fog->scale_transition, fog->scale, scale);
}
} // namespace nfs::game::world::fog
