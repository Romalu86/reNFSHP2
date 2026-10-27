#pragma once
#include "d3dx_matrix_dispatch.hpp"
namespace nfs::graphics::d3dx {
void __stdcall transform_point2_scalar(float *, const float *, const float *);
void __stdcall transform_point2_bootstrap(float *, const float *, const float *);
void __stdcall transform_point3_scalar(float *, const float *, const float *);
void __stdcall transform_point3_bootstrap(float *, const float *, const float *);
void __stdcall transform_vector4_scalar(float *, const float *, const float *);
void __stdcall transform_vector4_bootstrap(float *, const float *, const float *);
void __stdcall multiply_scalar(float *, const float *, const float *);
void __stdcall multiply_transposed_scalar(float *, const float *, const float *);
void __stdcall multiply_transposed_bootstrap(float *, const float *, const float *);
void __stdcall transform_normal2_scalar(float *, const float *, const float *);
void __stdcall transform_normal2_bootstrap(float *, const float *, const float *);
void __stdcall transform_normal3_scalar(float *, const float *, const float *);
void __stdcall transform_normal3_bootstrap(float *, const float *, const float *);
void __stdcall normalize_vector3_scalar(float *, const float *);
void __stdcall normalize_vector3_bootstrap(float *, const float *);
void __stdcall normalize_vector4_scalar(float *, const float *);
void __stdcall normalize_vector4_bootstrap(float *, const float *);
void __stdcall transform_coordinate2_scalar(float *, const float *, const float *);
void __stdcall transform_coordinate2_bootstrap(float *, const float *, const float *);
void __stdcall transform_coordinate3_scalar(float *, const float *, const float *);
void __stdcall transform_coordinate3_bootstrap(float *, const float *, const float *);
float __stdcall matrix_determinant_scalar(const float *);
float __stdcall matrix_determinant_bootstrap(const float *);
void __stdcall plane_from_point_normal_scalar(float *, const float *, const float *);
void __stdcall plane_from_point_normal_bootstrap(float *, const float *,
                                                 const float *);
void __stdcall plane_from_points_scalar(float *, const float *, const float *,
                                        const float *);
void __stdcall plane_from_points_bootstrap(float *, const float *, const float *,
                                           const float *);
float *__stdcall project_point3_scalar(float *, const float *, const Dword *, const float *,
                                       const float *, const float *);
float *__stdcall project_point3_bootstrap(float *, const float *, const Dword *, const float *,
                                          const float *, const float *);
float *__stdcall unproject_point3_scalar(float *, const float *, const Dword *, const float *,
                                         const float *, const float *);
float *__stdcall unproject_point3_bootstrap(float *, const float *, const Dword *, const float *,
                                            const float *, const float *);
void __stdcall cross_vector4_scalar(float *, const float *, const float *,
                                    const float *);
void __stdcall cross_vector4_bootstrap(float *, const float *, const float *,
                                       const float *);
void __stdcall hermite_vector2_scalar(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall hermite_vector2_bootstrap(float *, const float *, const float *, const float *,
                                         const float *, float);
float *__stdcall invert_matrix_scalar(float *, float *, const float *);
float *__stdcall invert_matrix_bootstrap(float *, float *, const float *);
float *__stdcall multiply_quaternion_scalar(float *, const float *, const float *);
float *__stdcall multiply_quaternion_bootstrap(float *, const float *, const float *);
void __stdcall normalize_quaternion_scalar(float *, const float *);
void __stdcall normalize_quaternion_bootstrap(float *, const float *);
void __stdcall normalize_plane_scalar(float *, const float *);
void __stdcall normalize_plane_bootstrap(float *, const float *);
float *__stdcall intersect_plane_line_scalar(float *, const float *, const float *,
                                             const float *);
float *__stdcall intersect_plane_line_bootstrap(float *, const float *, const float *,
                                                const float *);
void __stdcall quaternion_axis_angle_scalar(const float *, float *, float *);
void __stdcall quaternion_axis_angle_bootstrap(const float *, float *, float *);
void __stdcall quaternion_from_matrix_scalar(float *, const float *);
void __stdcall quaternion_from_matrix_bootstrap(float *, const float *);
void __stdcall quaternion_yaw_pitch_roll_scalar(float *, float, float, float);
void __stdcall quaternion_yaw_pitch_roll_bootstrap(float *, float, float, float);
void __stdcall translation_matrix_scalar(float *, float, float, float);
void __stdcall translation_matrix_bootstrap(float *, float, float, float);
void __stdcall shadow_matrix_scalar(float *, const float *, const float *);
void __stdcall shadow_matrix_bootstrap(float *, const float *, const float *);
void __stdcall reflection_matrix_scalar(float *, const float *);
void __stdcall reflection_matrix_bootstrap(float *, const float *);
float *__stdcall quaternion_from_axis_scalar(float *, const float *, float);
float *__stdcall quaternion_from_axis_bootstrap(float *, const float *, float);
void __stdcall rotation_x_matrix_scalar(float *, float);
void __stdcall rotation_x_matrix_bootstrap(float *, float);
void __stdcall rotation_y_matrix_scalar(float *, float);
void __stdcall rotation_y_matrix_bootstrap(float *, float);
void __stdcall rotation_z_matrix_scalar(float *, float);
void __stdcall rotation_z_matrix_bootstrap(float *, float);
void __stdcall scaling_matrix_scalar(float *, float, float, float);
void __stdcall scaling_matrix_bootstrap(float *, float, float, float);
void __stdcall transform_plane_scalar(float *, const float *, const float *);
void __stdcall transform_plane_bootstrap(float *, const float *, const float *);
void __stdcall interpolate_quaternion_scalar(float *, const float *, const float *,
                                             float);
void __stdcall interpolate_quaternion_bootstrap(float *, const float *, const float *,
                                                float);
float *__stdcall barycentric_quaternion_scalar(float *, const float *, const float *, const float *,
                                               float, float);
float *__stdcall barycentric_quaternion_bootstrap(float *, const float *, const float *,
                                                  const float *, float, float);
void __stdcall transpose_scalar(float *, const float *);
void __stdcall quaternion_matrix_scalar(float *, const float *);
void __stdcall quaternion_matrix_bootstrap(float *, const float *);
float *__stdcall transformation_matrix_scalar(float *, const float *, const float *, const float *,
                                              const float *, const float *,
                                              const float *);
float *__stdcall transformation_matrix_bootstrap(float *, const float *, const float *,
                                                 const float *, const float *, const float *,
                                                 const float *);
float *__stdcall yaw_pitch_roll_matrix_scalar(float *, float, float, float);
float *__stdcall yaw_pitch_roll_matrix_bootstrap(float *, float, float, float);
float *__stdcall affine_matrix_scalar(float *, float, const float *, const float *,
                                      const float *);
float *__stdcall affine_matrix_bootstrap(float *, float, const float *, const float *,
                                         const float *);
void __stdcall axis_rotation_matrix_scalar(float *, const float *, float);
void __stdcall axis_rotation_matrix_bootstrap(float *, const float *, float);
float *__stdcall squad_quaternion_scalar(float *, const float *, const float *, const float *,
                                         const float *, float);
float *__stdcall squad_quaternion_bootstrap(float *, const float *, const float *, const float *,
                                            const float *, float);
float *__stdcall inverse_quaternion_scalar(float *, const float *);
float *__stdcall inverse_quaternion_bootstrap(float *, const float *);
void __stdcall setup_squad_scalar(float *, float *, float *, const float *, const float *,
                                  const float *, const float *);
void __stdcall setup_squad_bootstrap(float *, float *, float *, const float *, const float *,
                                     const float *, const float *);
void __stdcall normalize_vector2_scalar(float *, const float *);
void __stdcall normalize_vector2_bootstrap(float *, const float *);
void __stdcall catmull_vector2_scalar(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall catmull_vector2_bootstrap(float *, const float *, const float *, const float *,
                                         const float *, float);
void __stdcall barycentric_vector2_scalar(float *, const float *, const float *, const float *,
                                          float, float);
void __stdcall barycentric_vector2_bootstrap(float *, const float *, const float *, const float *,
                                             float, float);
void __stdcall hermite_vector3_scalar(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall hermite_vector3_bootstrap(float *, const float *, const float *, const float *,
                                         const float *, float);
void __stdcall catmull_vector3_scalar(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall catmull_vector3_bootstrap(float *, const float *, const float *, const float *,
                                         const float *, float);
void __stdcall barycentric_vector3_scalar(float *, const float *, const float *, const float *,
                                          float, float);
void __stdcall barycentric_vector3_bootstrap(float *, const float *, const float *, const float *,
                                             float, float);
void __stdcall hermite_vector4_scalar(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall hermite_vector4_bootstrap(float *, const float *, const float *, const float *,
                                         const float *, float);
void __stdcall catmull_vector4_scalar(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall catmull_vector4_bootstrap(float *, const float *, const float *, const float *,
                                         const float *, float);
void __stdcall barycentric_vector4_scalar(float *, const float *, const float *, const float *,
                                          float, float);
void __stdcall barycentric_vector4_bootstrap(float *, const float *, const float *, const float *,
                                             float, float);
void __stdcall logarithm_quaternion_scalar(float *, const float *);
float *__stdcall exponential_quaternion_scalar(float *, const float *);
void __stdcall reflection_matrix_sse(float *, const float *);
void __stdcall transform_point2_sse(float *, const float *, const float *);
void __stdcall plane_from_points_sse(float *, const float *, const float *,
                                     const float *);
void __stdcall shadow_matrix_sse(float *, const float *, const float *);
float __stdcall matrix_determinant_sse(const float *);
void __stdcall barycentric_vector2_sse(float *, const float *, const float *, const float *, float,
                                       float);
void __stdcall catmull_vector2_sse(float *, const float *, const float *, const float *,
                                   const float *, float);
void __stdcall hermite_vector2_sse(float *, const float *, const float *, const float *,
                                   const float *, float);
void __stdcall barycentric_vector4_sse(float *, const float *, const float *, const float *, float,
                                       float);
void __stdcall catmull_vector4_sse(float *, const float *, const float *, const float *,
                                   const float *, float);
void __stdcall hermite_vector4_sse(float *, const float *, const float *, const float *,
                                   const float *, float);
void __stdcall barycentric_vector3_sse(float *, const float *, const float *, const float *, float,
                                       float);
void __stdcall catmull_vector3_sse(float *, const float *, const float *, const float *,
                                   const float *, float);
void __stdcall hermite_vector3_sse(float *, const float *, const float *, const float *,
                                   const float *, float);
float *__stdcall invert_matrix_sse(float *, float *, const float *);
float *__stdcall intersect_plane_line_sse(float *, const float *, const float *,
                                          const float *);
void __stdcall quaternion_yaw_pitch_roll_sse(float *, float, float, float);
void __stdcall quaternion_from_matrix_sse(float *, const float *);
float *__stdcall multiply_quaternion_sse(float *, const float *, const float *);
float *__stdcall inverse_quaternion_sse(float *, const float *);
void __stdcall normalize_quaternion_sse(float *, const float *);
void __stdcall cross_vector4_sse(float *, const float *, const float *, const float *);
void __stdcall normalize_plane_sse(float *, const float *);
void __stdcall normalize_vector4_sse(float *, const float *);
void __stdcall normalize_vector3_sse(float *, const float *);
void __stdcall transform_coordinate3_sse(float *, const float *, const float *);
void __stdcall transform_coordinate2_sse(float *, const float *, const float *);
void __stdcall transform_normal3_sse(float *, const float *, const float *);
void __stdcall transform_normal2_sse(float *, const float *, const float *);
void __stdcall multiply_transposed_sse(float *, const float *, const float *);
void __stdcall transform_vector4_sse(float *, const float *, const float *);
void __stdcall transform_point3_sse(float *, const float *, const float *);
void __stdcall reflection_matrix_sse2(float *, const float *);
void __stdcall plane_from_points_sse2(float *, const float *, const float *,
                                      const float *);
void __stdcall shadow_matrix_sse2(float *, const float *, const float *);
float __stdcall matrix_determinant_sse2(const float *);
float *__stdcall invert_matrix_sse2(float *, float *, const float *);
void __stdcall quaternion_yaw_pitch_roll_sse2(float *, float, float, float);
void __stdcall quaternion_from_matrix_sse2(float *, const float *);
void __stdcall rotation_y_matrix_sse2(float *, float);
void __stdcall rotation_x_matrix_sse2(float *, float);
void __stdcall cross_vector4_sse2(float *, const float *, const float *,
                                  const float *);
void __stdcall normalize_plane_sse2(float *, const float *);
void __stdcall normalize_vector3_sse2(float *, const float *);
void __stdcall transform_coordinate3_sse2(float *, const float *, const float *);
void __stdcall transform_normal3_sse2(float *, const float *, const float *);
void __stdcall multiply_transposed_sse2(float *, const float *, const float *);
void __stdcall transform_vector4_sse2(float *, const float *, const float *);
void __stdcall transform_point3_sse2(float *, const float *, const float *);
void __stdcall transform_point2_packed(float *, const float *, const float *);
void __stdcall transform_point3_packed(float *, const float *, const float *);
void __stdcall transform_vector4_packed(float *, const float *, const float *);
void __stdcall transform_normal2_packed(float *, const float *, const float *);
void __stdcall transform_normal3_packed(float *, const float *, const float *);
void __stdcall normalize_vector3_packed(float *, const float *);
void __stdcall normalize_vector4_packed(float *, const float *);
void __stdcall transform_coordinate2_packed(float *, const float *, const float *);
void __stdcall transform_coordinate3_packed(float *, const float *, const float *);
float __stdcall matrix_determinant_packed(const float *);
void __stdcall plane_from_point_normal_packed(float *, const float *, const float *);
void __stdcall plane_from_points_packed(float *, const float *, const float *,
                                        const float *);
float *__stdcall project_point3_packed(float *, const float *, const Dword *, const float *,
                                       const float *, const float *);
float *__stdcall unproject_point3_packed(float *, const float *, const Dword *, const float *,
                                         const float *, const float *);
void __stdcall cross_vector4_packed(float *, const float *, const float *,
                                    const float *);
void __stdcall hermite_vector2_packed(float *, const float *, const float *, const float *,
                                      const float *, float);
float *__stdcall invert_matrix_packed(float *, float *, const float *);
float *__stdcall multiply_quaternion_packed(float *, const float *, const float *);
void __stdcall normalize_quaternion_packed(float *, const float *);
void __stdcall normalize_plane_packed(float *, const float *);
float *__stdcall intersect_plane_line_packed(float *, const float *, const float *,
                                             const float *);
void __stdcall quaternion_axis_angle_packed(const float *, float *, float *);
void __stdcall quaternion_from_matrix_packed(float *, const float *);
void __stdcall quaternion_yaw_pitch_roll_packed(float *, float, float, float);
void __stdcall translation_matrix_packed(float *, float, float, float);
void __stdcall shadow_matrix_packed(float *, const float *, const float *);
void __stdcall reflection_matrix_packed(float *, const float *);
float *__stdcall quaternion_from_axis_packed(float *, const float *, float);
void __stdcall rotation_x_matrix_packed(float *, float);
void __stdcall rotation_y_matrix_packed(float *, float);
void __stdcall rotation_z_matrix_packed(float *, float);
void __stdcall scaling_matrix_packed(float *, float, float, float);
void __stdcall transform_plane_packed(float *, const float *, const float *);
void __stdcall interpolate_quaternion_packed(float *, const float *, const float *,
                                             float);
float *__stdcall barycentric_quaternion_packed(float *, const float *, const float *, const float *,
                                               float, float);
void __stdcall transpose_packed(float *, const float *);
void __stdcall quaternion_matrix_packed(float *, const float *);
float *__stdcall transformation_matrix_packed(float *, const float *, const float *, const float *,
                                              const float *, const float *,
                                              const float *);
float *__stdcall yaw_pitch_roll_matrix_packed(float *, float, float, float);
float *__stdcall affine_matrix_packed(float *, float, const float *, const float *,
                                      const float *);
void __stdcall axis_rotation_matrix_packed(float *, const float *, float);
float *__stdcall squad_quaternion_packed(float *, const float *, const float *, const float *,
                                         const float *, float);
float *__stdcall inverse_quaternion_packed(float *, const float *);
void __stdcall barycentric_vector4_packed(float *, const float *, const float *, const float *,
                                          float, float);
void __stdcall barycentric_vector3_packed(float *, const float *, const float *, const float *,
                                          float, float);
void __stdcall barycentric_vector2_packed(float *, const float *, const float *, const float *,
                                          float, float);
void __stdcall normalize_vector2_packed(float *, const float *);
void __stdcall hermite_vector4_packed(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall hermite_vector3_packed(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall logarithm_quaternion_packed(float *, const float *);
float *__stdcall exponential_quaternion_packed(float *, const float *);
void __stdcall catmull_vector4_packed(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall catmull_vector2_packed(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall catmull_vector3_packed(float *, const float *, const float *, const float *,
                                      const float *, float);
void __stdcall multiply_transposed_packed(float *, const float *, const float *);
void __stdcall setup_squad_packed(float *, float *, float *, const float *, const float *,
                                  const float *, const float *);
float *__stdcall unproject_point3_packed_extended(float *, const float *, const Dword *,
                                                  const float *, const float *,
                                                  const float *);
float *__stdcall invert_matrix_packed_extended(float *, float *, const float *);
void __stdcall quaternion_matrix_packed_extended(float *, const float *);
float *__stdcall transformation_matrix_packed_extended(float *, const float *, const float *,
                                                       const float *, const float *, const float *,
                                                       const float *);
void __stdcall quaternion_from_matrix_packed_extended(float *, const float *);
Dword packed_processor_capabilities();
int approximately_equal(float, float);
} // namespace nfs::graphics::d3dx
