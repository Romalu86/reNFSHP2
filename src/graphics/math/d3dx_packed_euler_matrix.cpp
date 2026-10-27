#include "d3dx_packed_matrix_values.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {

float *__stdcall yaw_pitch_roll_matrix_packed(float *out, float yaw, float pitch, float roll) {
    float rotation[4];
    packed::yaw_pitch_roll_values(rotation, yaw, pitch, roll);
    packed::quaternion_matrix_values(out, rotation);
    _m_femms();
    return out;
}
} // namespace nfs::graphics::d3dx
