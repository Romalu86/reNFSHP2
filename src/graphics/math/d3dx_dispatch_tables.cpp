#include "d3dx_dispatch_kernels.hpp"
namespace nfs::graphics::d3dx {

void __stdcall transform_point2_bootstrap(float *output, const float *input, const float *matrix) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[0])(output, input, matrix);
}

void __stdcall transform_point3_bootstrap(float *output, const float *input, const float *matrix) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[1])(output, input, matrix);
}

void __stdcall transform_vector4_bootstrap(float *output, const float *input, const float *matrix) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[2])(output, input, matrix);
}

void __stdcall multiply_transposed_bootstrap(float *output, const float *input,
                                             const float *matrix) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[4])(output, input, matrix);
}

void __stdcall transform_normal2_bootstrap(float *output, const float *input, const float *matrix) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[5])(output, input, matrix);
}

void __stdcall transform_normal3_bootstrap(float *output, const float *input, const float *matrix) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[6])(output, input, matrix);
}

void __stdcall normalize_vector3_bootstrap(float *output, const float *input) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[7])(output, input);
}

void __stdcall normalize_vector4_bootstrap(float *output, const float *input) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[8])(output, input);
}

void __stdcall transform_coordinate2_bootstrap(float *output, const float *input,
                                               const float *matrix) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[9])(output, input, matrix);
}

void __stdcall transform_coordinate3_bootstrap(float *output, const float *input,
                                               const float *matrix) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[10])(output, input, matrix);
}

float __stdcall matrix_determinant_bootstrap(const float *matrix) {
    pending::initialize_dispatch(1);
    using Callback = float(__stdcall *)(const float *);
    return reinterpret_cast<Callback>(pending::dispatch[11])(matrix);
}

void __stdcall plane_from_point_normal_bootstrap(float *plane, const float *point,
                                                 const float *normal) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[12])(plane, point, normal);
}

void __stdcall plane_from_points_bootstrap(float *plane, const float *first_point,
                                           const float *second_point, const float *third_point) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[13])(plane, first_point, second_point,
                                                      third_point);
}

float *__stdcall project_point3_bootstrap(float *output, const float *input, const Dword *viewport,
                                          const float *projection, const float *view,
                                          const float *world) {
    pending::initialize_dispatch(1);
    using Callback = float *(__stdcall *)(float *, const float *, const Dword *, const float *,
                                          const float *, const float *);
    return reinterpret_cast<Callback>(pending::dispatch[14])(output, input, viewport, projection,
                                                             view, world);
}

float *__stdcall unproject_point3_bootstrap(float *output, const float *input, const Dword *viewport,
                                            const float *projection, const float *view,
                                            const float *world) {
    pending::initialize_dispatch(1);
    using Callback = float *(__stdcall *)(float *, const float *, const Dword *, const float *,
                                          const float *, const float *);
    return reinterpret_cast<Callback>(pending::dispatch[15])(output, input, viewport, projection,
                                                             view, world);
}

void __stdcall cross_vector4_bootstrap(float *output, const float *first, const float *second,
                                       const float *third) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[16])(output, first, second, third);
}

void __stdcall hermite_vector2_bootstrap(float *output, const float *first,
                                         const float *first_tangent, const float *second,
                                         const float *second_tangent, float time) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *, const float *,
                                       const float *, float);
    reinterpret_cast<Callback>(pending::dispatch[17])(output, first, first_tangent, second,
                                                      second_tangent, time);
}

float *__stdcall invert_matrix_bootstrap(float *output, float *determinant, const float *input) {
    pending::initialize_dispatch(1);
    using Callback = float *(__stdcall *)(float *, float *, const float *);
    return reinterpret_cast<Callback>(pending::dispatch[18])(output, determinant, input);
}

float *__stdcall multiply_quaternion_bootstrap(float *output, const float *first,
                                               const float *second) {
    pending::initialize_dispatch(1);
    using Callback = float *(__stdcall *)(float *, const float *, const float *);
    return reinterpret_cast<Callback>(pending::dispatch[19])(output, first, second);
}

void __stdcall normalize_quaternion_bootstrap(float *output, const float *input) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[20])(output, input);
}

void __stdcall normalize_plane_bootstrap(float *output, const float *input) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[21])(output, input);
}

float *__stdcall intersect_plane_line_bootstrap(float *output, const float *plane,
                                                const float *first, const float *second) {
    pending::initialize_dispatch(1);
    using Callback = float *(__stdcall *)(float *, const float *, const float *, const float *);
    return reinterpret_cast<Callback>(pending::dispatch[22])(output, plane, first, second);
}

void __stdcall quaternion_axis_angle_bootstrap(const float *rotation, float *axis, float *angle) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(const float *, float *, float *);
    reinterpret_cast<Callback>(pending::dispatch[23])(rotation, axis, angle);
}

void __stdcall quaternion_from_matrix_bootstrap(float *output, const float *input) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[24])(output, input);
}

void __stdcall quaternion_yaw_pitch_roll_bootstrap(float *output, float yaw, float pitch,
                                                   float roll) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, float, float, float);
    reinterpret_cast<Callback>(pending::dispatch[25])(output, yaw, pitch, roll);
}

void __stdcall translation_matrix_bootstrap(float *output, float x, float y, float z) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, float, float, float);
    reinterpret_cast<Callback>(pending::dispatch[26])(output, x, y, z);
}

void __stdcall shadow_matrix_bootstrap(float *output, const float *light, const float *plane) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[27])(output, light, plane);
}

void __stdcall reflection_matrix_bootstrap(float *output, const float *input) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[28])(output, input);
}

float *__stdcall quaternion_from_axis_bootstrap(float *output, const float *axis, float angle) {
    pending::initialize_dispatch(1);
    using Callback = float *(__stdcall *)(float *, const float *, float);
    return reinterpret_cast<Callback>(pending::dispatch[29])(output, axis, angle);
}

void __stdcall rotation_x_matrix_bootstrap(float *output, float angle) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, float);
    reinterpret_cast<Callback>(pending::dispatch[30])(output, angle);
}

void __stdcall rotation_y_matrix_bootstrap(float *output, float angle) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, float);
    reinterpret_cast<Callback>(pending::dispatch[31])(output, angle);
}

void __stdcall rotation_z_matrix_bootstrap(float *output, float angle) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, float);
    reinterpret_cast<Callback>(pending::dispatch[32])(output, angle);
}

void __stdcall scaling_matrix_bootstrap(float *output, float x, float y, float z) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, float, float, float);
    reinterpret_cast<Callback>(pending::dispatch[33])(output, x, y, z);
}

void __stdcall transform_plane_bootstrap(float *output, const float *input, const float *matrix) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[34])(output, input, matrix);
}

void __stdcall interpolate_quaternion_bootstrap(float *output, const float *first,
                                                const float *second, float time) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *, float);
    reinterpret_cast<Callback>(pending::dispatch[35])(output, first, second, time);
}

float *__stdcall barycentric_quaternion_bootstrap(float *output, const float *origin,
                                                  const float *first, const float *second,
                                                  float first_weight, float second_weight) {
    pending::initialize_dispatch(1);
    using Callback =
        float *(__stdcall *)(float *, const float *, const float *, const float *, float, float);
    return reinterpret_cast<Callback>(pending::dispatch[36])(output, origin, first, second,
                                                             first_weight, second_weight);
}

void __stdcall quaternion_matrix_bootstrap(float *output, const float *input) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[38])(output, input);
}

float *__stdcall transformation_matrix_bootstrap(float *output, const float *scaling_center,
                                                 const float *scaling_rotation,
                                                 const float *scaling, const float *rotation_center,
                                                 const float *rotation, const float *translation) {
    pending::initialize_dispatch(1);
    using Callback = float *(__stdcall *)(float *, const float *, const float *, const float *,
                                          const float *, const float *, const float *);
    return reinterpret_cast<Callback>(pending::dispatch[39])(
        output, scaling_center, scaling_rotation, scaling, rotation_center, rotation, translation);
}

float *__stdcall yaw_pitch_roll_matrix_bootstrap(float *output, float yaw, float pitch,
                                                 float roll) {
    pending::initialize_dispatch(1);
    using Callback = float *(__stdcall *)(float *, float, float, float);
    return reinterpret_cast<Callback>(pending::dispatch[40])(output, yaw, pitch, roll);
}

float *__stdcall affine_matrix_bootstrap(float *output, float scale, const float *rotation_center,
                                         const float *rotation, const float *translation) {
    pending::initialize_dispatch(1);
    using Callback =
        float *(__stdcall *)(float *, float, const float *, const float *, const float *);
    return reinterpret_cast<Callback>(pending::dispatch[41])(output, scale, rotation_center,
                                                             rotation, translation);
}

void __stdcall axis_rotation_matrix_bootstrap(float *output, const float *axis, float angle) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, float);
    reinterpret_cast<Callback>(pending::dispatch[42])(output, axis, angle);
}

float *__stdcall squad_quaternion_bootstrap(float *output, const float *first,
                                            const float *control_first, const float *control_second,
                                            const float *second, float time) {
    pending::initialize_dispatch(1);
    using Callback = float *(__stdcall *)(float *, const float *, const float *, const float *,
                                          const float *, float);
    return reinterpret_cast<Callback>(pending::dispatch[43])(output, first, control_first,
                                                             control_second, second, time);
}

float *__stdcall inverse_quaternion_bootstrap(float *output, const float *input) {
    pending::initialize_dispatch(1);
    using Callback = float *(__stdcall *)(float *, const float *);
    return reinterpret_cast<Callback>(pending::dispatch[44])(output, input);
}

void __stdcall setup_squad_bootstrap(float *first_control, float *second_control,
                                     float *third_control, const float *previous,
                                     const float *first, const float *second, const float *next) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, float *, float *, const float *, const float *,
                                       const float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[45])(first_control, second_control, third_control,
                                                      previous, first, second, next);
}

void __stdcall normalize_vector2_bootstrap(float *output, const float *input) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *);
    reinterpret_cast<Callback>(pending::dispatch[46])(output, input);
}

void __stdcall catmull_vector2_bootstrap(float *output, const float *first, const float *second,
                                         const float *third, const float *fourth, float time) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *, const float *,
                                       const float *, float);
    reinterpret_cast<Callback>(pending::dispatch[47])(output, first, second, third, fourth, time);
}

void __stdcall barycentric_vector2_bootstrap(float *output, const float *origin, const float *first,
                                             const float *second, float first_weight,
                                             float second_weight) {
    pending::initialize_dispatch(1);
    using Callback =
        void(__stdcall *)(float *, const float *, const float *, const float *, float, float);
    reinterpret_cast<Callback>(pending::dispatch[48])(output, origin, first, second, first_weight,
                                                      second_weight);
}

void __stdcall hermite_vector3_bootstrap(float *output, const float *first,
                                         const float *first_tangent, const float *second,
                                         const float *second_tangent, float time) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *, const float *,
                                       const float *, float);
    reinterpret_cast<Callback>(pending::dispatch[49])(output, first, first_tangent, second,
                                                      second_tangent, time);
}

void __stdcall catmull_vector3_bootstrap(float *output, const float *first, const float *second,
                                         const float *third, const float *fourth, float time) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *, const float *,
                                       const float *, float);
    reinterpret_cast<Callback>(pending::dispatch[50])(output, first, second, third, fourth, time);
}

void __stdcall barycentric_vector3_bootstrap(float *output, const float *origin, const float *first,
                                             const float *second, float first_weight,
                                             float second_weight) {
    pending::initialize_dispatch(1);
    using Callback =
        void(__stdcall *)(float *, const float *, const float *, const float *, float, float);
    reinterpret_cast<Callback>(pending::dispatch[51])(output, origin, first, second, first_weight,
                                                      second_weight);
}

void __stdcall hermite_vector4_bootstrap(float *output, const float *first,
                                         const float *first_tangent, const float *second,
                                         const float *second_tangent, float time) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *, const float *,
                                       const float *, float);
    reinterpret_cast<Callback>(pending::dispatch[52])(output, first, first_tangent, second,
                                                      second_tangent, time);
}

void __stdcall catmull_vector4_bootstrap(float *output, const float *first, const float *second,
                                         const float *third, const float *fourth, float time) {
    pending::initialize_dispatch(1);
    using Callback = void(__stdcall *)(float *, const float *, const float *, const float *,
                                       const float *, float);
    reinterpret_cast<Callback>(pending::dispatch[53])(output, first, second, third, fourth, time);
}

void __stdcall barycentric_vector4_bootstrap(float *output, const float *origin, const float *first,
                                             const float *second, float first_weight,
                                             float second_weight) {
    pending::initialize_dispatch(1);
    using Callback =
        void(__stdcall *)(float *, const float *, const float *, const float *, float, float);
    reinterpret_cast<Callback>(pending::dispatch[54])(output, origin, first, second, first_weight,
                                                      second_weight);
}
namespace pending {

Dword dispatch[57] = {
    Dword(&transform_point2_bootstrap),
    Dword(&transform_point3_bootstrap),
    Dword(&transform_vector4_bootstrap),
    Dword(&multiply_bootstrap),
    Dword(&multiply_transposed_bootstrap),
    Dword(&transform_normal2_bootstrap),
    Dword(&transform_normal3_bootstrap),
    Dword(&normalize_vector3_bootstrap),
    Dword(&normalize_vector4_bootstrap),
    Dword(&transform_coordinate2_bootstrap),
    Dword(&transform_coordinate3_bootstrap),
    Dword(&matrix_determinant_bootstrap),
    Dword(&plane_from_point_normal_bootstrap),
    Dword(&plane_from_points_bootstrap),
    Dword(&project_point3_bootstrap),
    Dword(&unproject_point3_bootstrap),
    Dword(&cross_vector4_bootstrap),
    Dword(&hermite_vector2_bootstrap),
    Dword(&invert_matrix_bootstrap),
    Dword(&multiply_quaternion_bootstrap),
    Dword(&normalize_quaternion_bootstrap),
    Dword(&normalize_plane_bootstrap),
    Dword(&intersect_plane_line_bootstrap),
    Dword(&quaternion_axis_angle_bootstrap),
    Dword(&quaternion_from_matrix_bootstrap),
    Dword(&quaternion_yaw_pitch_roll_bootstrap),
    Dword(&translation_matrix_bootstrap),
    Dword(&shadow_matrix_bootstrap),
    Dword(&reflection_matrix_bootstrap),
    Dword(&quaternion_from_axis_bootstrap),
    Dword(&rotation_x_matrix_bootstrap),
    Dword(&rotation_y_matrix_bootstrap),
    Dword(&rotation_z_matrix_bootstrap),
    Dword(&scaling_matrix_bootstrap),
    Dword(&transform_plane_bootstrap),
    Dword(&interpolate_quaternion_bootstrap),
    Dword(&barycentric_quaternion_bootstrap),
    Dword(&transpose_bootstrap),
    Dword(&quaternion_matrix_bootstrap),
    Dword(&transformation_matrix_bootstrap),
    Dword(&yaw_pitch_roll_matrix_bootstrap),
    Dword(&affine_matrix_bootstrap),
    Dword(&axis_rotation_matrix_bootstrap),
    Dword(&squad_quaternion_bootstrap),
    Dword(&inverse_quaternion_bootstrap),
    Dword(&setup_squad_bootstrap),
    Dword(&normalize_vector2_bootstrap),
    Dword(&catmull_vector2_bootstrap),
    Dword(&barycentric_vector2_bootstrap),
    Dword(&hermite_vector3_bootstrap),
    Dword(&catmull_vector3_bootstrap),
    Dword(&barycentric_vector3_bootstrap),
    Dword(&hermite_vector4_bootstrap),
    Dword(&catmull_vector4_bootstrap),
    Dword(&barycentric_vector4_bootstrap),
    0,
    0,
};

const Dword default_dispatch[57] = {
    Dword(&transform_point2_scalar),        Dword(&transform_point3_scalar),
    Dword(&transform_vector4_scalar),       Dword(&multiply_scalar),
    Dword(&multiply_transposed_scalar),     Dword(&transform_normal2_scalar),
    Dword(&transform_normal3_scalar),       Dword(&normalize_vector3_scalar),
    Dword(&normalize_vector4_scalar),       Dword(&transform_coordinate2_scalar),
    Dword(&transform_coordinate3_scalar),   Dword(&matrix_determinant_scalar),
    Dword(&plane_from_point_normal_scalar), Dword(&plane_from_points_scalar),
    Dword(&project_point3_scalar),          Dword(&unproject_point3_scalar),
    Dword(&cross_vector4_scalar),           Dword(&hermite_vector2_scalar),
    Dword(&invert_matrix_scalar),           Dword(&multiply_quaternion_scalar),
    Dword(&normalize_quaternion_scalar),    Dword(&normalize_plane_scalar),
    Dword(&intersect_plane_line_scalar),    Dword(&quaternion_axis_angle_scalar),
    Dword(&quaternion_from_matrix_scalar),  Dword(&quaternion_yaw_pitch_roll_scalar),
    Dword(&translation_matrix_scalar),      Dword(&shadow_matrix_scalar),
    Dword(&reflection_matrix_scalar),       Dword(&quaternion_from_axis_scalar),
    Dword(&rotation_x_matrix_scalar),       Dword(&rotation_y_matrix_scalar),
    Dword(&rotation_z_matrix_scalar),       Dword(&scaling_matrix_scalar),
    Dword(&transform_plane_scalar),         Dword(&interpolate_quaternion_scalar),
    Dword(&barycentric_quaternion_scalar),  Dword(&transpose_scalar),
    Dword(&quaternion_matrix_scalar),       Dword(&transformation_matrix_scalar),
    Dword(&yaw_pitch_roll_matrix_scalar),   Dword(&affine_matrix_scalar),
    Dword(&axis_rotation_matrix_scalar),    Dword(&squad_quaternion_scalar),
    Dword(&inverse_quaternion_scalar),      Dword(&setup_squad_scalar),
    Dword(&normalize_vector2_scalar),       Dword(&catmull_vector2_scalar),
    Dword(&barycentric_vector2_scalar),     Dword(&hermite_vector3_scalar),
    Dword(&catmull_vector3_scalar),         Dword(&barycentric_vector3_scalar),
    Dword(&hermite_vector4_scalar),         Dword(&catmull_vector4_scalar),
    Dword(&barycentric_vector4_scalar),     Dword(&logarithm_quaternion_scalar),
    Dword(&exponential_quaternion_scalar),
};
} // namespace pending
} // namespace nfs::graphics::d3dx
