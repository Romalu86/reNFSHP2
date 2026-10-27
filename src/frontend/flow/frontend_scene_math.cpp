#include "frontend_scene_widgets.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../ai/driving/ai_route_planner.hpp"
#include <cmath>
namespace nfs::application::frontend {
using kernel::numeric::Extended;

void rotate_scene_basis_z(const float *input, float angle, float *output) {
    float scratch[9];
    if (input == output) {
        for (unsigned index = 0; index < 9; ++index)
            scratch[index] = input[index];
        input = scratch;
    }
    float sine, cosine;
    game::runtime::turn_sine_cosine(&angle, &sine, &cosine);
    const auto format = kernel::numeric::current_format();
    for (unsigned index = 0; index < 3; ++index)
        output[index] = float(Extended(cosine, format) * input[index] +
                              Extended(sine, format) * input[index + 3]);
    for (unsigned index = 0; index < 3; ++index)
        output[index + 3] = float(Extended(cosine, format) * input[index + 3] -
                                  Extended(sine, format) * input[index]);
    output[6] = input[6];
    output[7] = input[7];
    output[8] = input[8];
}

void scene_euler_matrix(float x, float y, float z, float *output) {
    float intermediate[9];
    game::runtime::matrices::rotate_x(output, x);
    game::runtime::ai::lifecycle::rotate_y_basis(output, y, intermediate);
    rotate_scene_basis_z(intermediate, z, output);
}

void scene_euler_angles(const float *matrix, float *x, float *y, float *z) {
    const auto format = kernel::numeric::current_format();
    const float vertical = -matrix[6];
    const float remainder = float(Extended(1.f, format) - Extended(vertical, format) * vertical);
    const auto magnitude = Extended(remainder, format).square_root();
    float xx, xy, zx, zz;
    if (float(magnitude) != 0 || std::isnan(float(magnitude))) {
        const auto inverse = Extended(1.f, format) / Extended(float(magnitude), format);
        xy = float(inverse * matrix[7]);
        xx = float(inverse * matrix[8]);
        zx = float(inverse * matrix[3]);
        zz = float(inverse * matrix[0]);
    } else {
        xx = matrix[4];
        zx = 0;
        xy = -matrix[5];
        zz = 1;
    }
    auto store = [&](float first, float second, float *destination) {
        const auto turns = -game::runtime::ai::entities::pending::direction_turns(first, second);
        *destination = float(turns);
        if (turns < Extended(0.f, format))
            *destination = float(turns + 1.f);
    };
    store(xy, xx, x);
    store(vertical, float(magnitude), y);
    store(zx, zz, z);
}
} // namespace nfs::application::frontend
