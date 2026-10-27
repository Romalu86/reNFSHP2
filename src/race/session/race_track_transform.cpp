#include "race_world_draw.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::world::track_render {

void compose_track_transform(float *matrix, float scale_x, float scale_y, float scale_z,
                             float angle_x, float angle_y, float angle_z,
                             float x, float y, float z) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    angle_x = float(E(angle_x) * 0.0027777778f);
    angle_y = float(E(angle_y) * 0.0027777778f);
    angle_z = float(E(angle_z) * 0.0027777778f);
    const float sine_x = float(runtime::turn_sine(angle_x));
    const float cosine_x = float(runtime::turn_cosine(angle_x));
    const float sine_y = float(runtime::turn_sine(angle_y));
    const float cosine_y = float(runtime::turn_cosine(angle_y));
    const float sine_z = float(runtime::turn_sine(angle_z));
    const auto cosine_z = Extended::from_double(double(runtime::turn_cosine(angle_z)), format);
    matrix[3] = matrix[7] = 0;
    matrix[0] = float(E(cosine_y) * cosine_z * scale_x);
    matrix[1] = float(E(sine_z) * cosine_y * scale_x);
    matrix[2] = float(-(E(sine_y) * scale_x));
    const auto sine_y_cosine_z = E(sine_y) * cosine_z;
    matrix[4] = float((E(sine_x) * sine_y_cosine_z - E(sine_z) * cosine_x) * scale_y);
    const auto sine_z_sine_y = E(sine_z) * sine_y;
    const float rounded_product = float(sine_z_sine_y);
    matrix[5] = float((sine_z_sine_y * sine_x + cosine_z * cosine_x) * scale_y);
    matrix[6] = float(E(cosine_y) * sine_x * scale_y);
    matrix[8] = float((sine_y_cosine_z * cosine_x + E(sine_z) * sine_x) * scale_z);
    const auto vertical = E(rounded_product) * cosine_x - cosine_z * sine_x;
    matrix[11] = 0;
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
    matrix[15] = 1;
    matrix[9] = float(vertical * scale_z);
    matrix[10] = float(E(cosine_y) * cosine_x * scale_z);
}
} // namespace nfs::game::world::track_render
