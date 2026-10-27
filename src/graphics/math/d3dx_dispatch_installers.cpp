#include "d3dx_dispatch_kernels.hpp"
namespace nfs::graphics::d3dx::pending {

void install_sse(Dword *table) {
    table[28] = Dword(&reflection_matrix_sse);
    table[0] = Dword(&transform_point2_sse);
    table[13] = Dword(&plane_from_points_sse);
    table[27] = Dword(&shadow_matrix_sse);
    table[11] = Dword(&matrix_determinant_sse);
    table[48] = Dword(&barycentric_vector2_sse);
    table[47] = Dword(&catmull_vector2_sse);
    table[17] = Dword(&hermite_vector2_sse);
    table[54] = Dword(&barycentric_vector4_sse);
    table[53] = Dword(&catmull_vector4_sse);
    table[52] = Dword(&hermite_vector4_sse);
    table[51] = Dword(&barycentric_vector3_sse);
    table[50] = Dword(&catmull_vector3_sse);
    table[49] = Dword(&hermite_vector3_sse);
    table[18] = Dword(&invert_matrix_sse);
    table[22] = Dword(&intersect_plane_line_sse);
    table[25] = Dword(&quaternion_yaw_pitch_roll_sse);
    table[24] = Dword(&quaternion_from_matrix_sse);
    table[19] = Dword(&multiply_quaternion_sse);
    table[44] = Dword(&inverse_quaternion_sse);
    table[20] = Dword(&normalize_quaternion_sse);
    table[16] = Dword(&cross_vector4_sse);
    table[21] = Dword(&normalize_plane_sse);
    table[8] = Dword(&normalize_vector4_sse);
    table[7] = Dword(&normalize_vector3_sse);
    table[10] = Dword(&transform_coordinate3_sse);
    table[9] = Dword(&transform_coordinate2_sse);
    table[6] = Dword(&transform_normal3_sse);
    table[5] = Dword(&transform_normal2_sse);
    table[4] = Dword(&multiply_transposed_sse);
    table[3] = Dword(&multiply_sse);
    table[2] = Dword(&transform_vector4_sse);
    table[1] = Dword(&transform_point3_sse);
}

void install_sse2(Dword *table) {
    table[28] = Dword(&reflection_matrix_sse2);
    table[0] = Dword(&transform_point2_sse);
    table[13] = Dword(&plane_from_points_sse2);
    table[27] = Dword(&shadow_matrix_sse2);
    table[11] = Dword(&matrix_determinant_sse2);
    table[48] = Dword(&barycentric_vector2_sse);
    table[47] = Dword(&catmull_vector2_sse);
    table[17] = Dword(&hermite_vector2_sse);
    table[54] = Dword(&barycentric_vector4_sse);
    table[53] = Dword(&catmull_vector4_sse);
    table[52] = Dword(&hermite_vector4_sse);
    table[51] = Dword(&barycentric_vector3_sse);
    table[50] = Dword(&catmull_vector3_sse);
    table[49] = Dword(&hermite_vector3_sse);
    table[18] = Dword(&invert_matrix_sse2);
    table[22] = Dword(&intersect_plane_line_sse);
    table[25] = Dword(&quaternion_yaw_pitch_roll_sse2);
    table[24] = Dword(&quaternion_from_matrix_sse2);
    table[31] = Dword(&rotation_y_matrix_sse2);
    table[30] = Dword(&rotation_x_matrix_sse2);
    table[19] = Dword(&multiply_quaternion_sse);
    table[20] = Dword(&normalize_quaternion_sse);
    table[16] = Dword(&cross_vector4_sse2);
    table[21] = Dword(&normalize_plane_sse2);
    table[8] = Dword(&normalize_vector4_sse);
    table[7] = Dword(&normalize_vector3_sse2);
    table[10] = Dword(&transform_coordinate3_sse2);
    table[9] = Dword(&transform_coordinate2_sse);
    table[6] = Dword(&transform_normal3_sse2);
    table[5] = Dword(&transform_normal2_sse);
    table[4] = Dword(&multiply_transposed_sse2);
    table[3] = Dword(&multiply_sse2);
    table[2] = Dword(&transform_vector4_sse2);
    table[1] = Dword(&transform_point3_sse2);
}

void install_packed(Dword *table) {
    Dword features = packed_processor_capabilities();
    if (!(features & 32) || !(features & 128))
        return;
    table[0] = Dword(&transform_point2_packed);
    table[1] = Dword(&transform_point3_packed);
    table[2] = Dword(&transform_vector4_packed);
    table[3] = Dword(&multiply_packed);
    table[5] = Dword(&transform_normal2_packed);
    table[6] = Dword(&transform_normal3_packed);
    table[7] = Dword(&normalize_vector3_packed);
    table[8] = Dword(&normalize_vector4_packed);
    table[9] = Dword(&transform_coordinate2_packed);
    table[10] = Dword(&transform_coordinate3_packed);
    table[11] = Dword(&matrix_determinant_packed);
    table[12] = Dword(&plane_from_point_normal_packed);
    table[13] = Dword(&plane_from_points_packed);
    table[14] = Dword(&project_point3_packed);
    table[15] = Dword(&unproject_point3_packed);
    table[16] = Dword(&cross_vector4_packed);
    table[17] = Dword(&hermite_vector2_packed);
    table[18] = Dword(&invert_matrix_packed);
    table[19] = Dword(&multiply_quaternion_packed);
    table[20] = Dword(&normalize_quaternion_packed);
    table[21] = Dword(&normalize_plane_packed);
    table[22] = Dword(&intersect_plane_line_packed);
    table[23] = Dword(&quaternion_axis_angle_packed);
    table[24] = Dword(&quaternion_from_matrix_packed);
    table[25] = Dword(&quaternion_yaw_pitch_roll_packed);
    table[26] = Dword(&translation_matrix_packed);
    table[27] = Dword(&shadow_matrix_packed);
    table[28] = Dword(&reflection_matrix_packed);
    table[29] = Dword(&quaternion_from_axis_packed);
    table[30] = Dword(&rotation_x_matrix_packed);
    table[31] = Dword(&rotation_y_matrix_packed);
    table[32] = Dword(&rotation_z_matrix_packed);
    table[33] = Dword(&scaling_matrix_packed);
    table[34] = Dword(&transform_plane_packed);
    table[35] = Dword(&interpolate_quaternion_packed);
    table[36] = Dword(&barycentric_quaternion_packed);
    table[37] = Dword(&transpose_packed);
    table[38] = Dword(&quaternion_matrix_packed);
    table[39] = Dword(&transformation_matrix_packed);
    table[40] = Dword(&yaw_pitch_roll_matrix_packed);
    table[41] = Dword(&affine_matrix_packed);
    table[42] = Dword(&axis_rotation_matrix_packed);
    table[43] = Dword(&squad_quaternion_packed);
    table[44] = Dword(&inverse_quaternion_packed);
    table[54] = Dword(&barycentric_vector4_packed);
    table[51] = Dword(&barycentric_vector3_packed);
    table[48] = Dword(&barycentric_vector2_packed);
    table[46] = Dword(&normalize_vector2_packed);
    table[52] = Dword(&hermite_vector4_packed);
    table[49] = Dword(&hermite_vector3_packed);
    table[55] = Dword(&logarithm_quaternion_packed);
    table[56] = Dword(&exponential_quaternion_packed);
    table[53] = Dword(&catmull_vector4_packed);
    table[47] = Dword(&catmull_vector2_packed);
    table[50] = Dword(&catmull_vector3_packed);
    table[4] = Dword(&multiply_transposed_packed);
    table[45] = Dword(&setup_squad_packed);
    if ((features & 256) && (features & 512)) {
        table[15] = Dword(&unproject_point3_packed_extended);
        table[18] = Dword(&invert_matrix_packed_extended);
        table[38] = Dword(&quaternion_matrix_packed_extended);
        table[39] = Dword(&transformation_matrix_packed_extended);
        table[24] = Dword(&quaternion_from_matrix_packed_extended);
    }
}
} // namespace nfs::graphics::d3dx::pending
