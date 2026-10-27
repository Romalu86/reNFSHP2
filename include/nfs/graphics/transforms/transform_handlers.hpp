#pragma once
#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::game::runtime::matrices::handlers {
extern float near_plane, depth_scale, screen_x, screen_y, offset_x, offset_y, reciprocal_scale;
extern Dword clip_intersection, clip_union;
extern void *clip_cursor, *copy_cursor, *repeat_cursor, *repeat_flags_cursor;
extern float null_translation[4];
extern Dword clip_limits[8];
extern Dword prepared_matrix[12], prepared_vectors[12], projection_changed;
void prepare_packed();
void prepare_streaming();
void translate(int, const void *, void *);
void translate_indexed(int, const void *, void *);
void project(int, const void *, void *);
void project_indexed(int, const void *, void *);
void translate_project(int, const void *, void *);
void translate_project_indexed(int, const void *, void *);
void capture_project(int, const void *, void *);
void capture_translate_project(int, const void *, void *);
void capture_rotate_project(int, const void *, void *);
void capture_affine_project(int, const void *, void *);
void capture_project_indexed(int, const void *, void *);
void capture_translate_project_indexed(int, const void *, void *);
void capture_rotate_project_indexed(int, const void *, void *);
void capture_affine_project_indexed(int, const void *, void *);
void rotate(int, const void *, void *);
void affine(int, const void *, void *);
void rotate_project(int, const void *, void *);
void affine_project(int, const void *, void *);
void rotate_indexed(int, const void *, void *);
void affine_indexed(int, const void *, void *);
void rotate_project_indexed(int, const void *, void *);
void affine_project_indexed(int, const void *, void *);
void clip(int, void *);
void clip_indexed(int, void *);
void clip_and_repeat(int, void *);
void repeat(int, void *);
void clip_vertex(void *, Byte *);
void scalar_affine(int, const void *, void *, const float *);
void scalar_affine_project(int, const void *, void *, const float *);
void scalar_affine_indexed(int, const void *, void *, const float *);
void scalar_affine_project_indexed(int, const void *, void *, const float *);
namespace pending {
void packed_affine(int, const void *, void *, const float *);
void packed_affine_project(int, const void *, void *, const float *);
void packed_affine_indexed(int, const void *, void *, const float *);
void packed_affine_project_indexed(int, const void *, void *, const float *);
void streaming_affine(int, const void *, void *, const float *);
void streaming_affine_project(int, const void *, void *, const float *);
void streaming_affine_indexed(int, const void *, void *, const float *);
void streaming_affine_project_indexed(int, const void *, void *, const float *);
} // namespace pending
} // namespace nfs::game::runtime::matrices::handlers
