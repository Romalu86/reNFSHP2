#pragma once
#include "../actors/model_render_runtime.hpp"
namespace nfs::graphics::d3dx {
using Multiply = float *(__stdcall *)(float *, const float *, const float *);
using Transpose = void(__stdcall *)(float *, const float *);
extern Multiply &matrix_multiply;
extern Transpose &matrix_transpose;
float *__stdcall multiply_bootstrap(float *, const float *, const float *);
void __stdcall transpose_bootstrap(float *, const float *);
float *__stdcall multiply_baseline(float *, const float *, const float *);
float *__stdcall multiply_sse(float *, const float *, const float *);
float *__stdcall multiply_sse2(float *, const float *, const float *);
float *__stdcall multiply_packed(float *, const float *, const float *);
namespace pending {
int initialize_dispatch(int);
extern Dword dispatch[57];
extern const Dword default_dispatch[57];
void install_baseline(Dword *);
void install_sse(Dword *);
void install_sse2(Dword *);
void install_packed(Dword *);
} // namespace pending
} // namespace nfs::graphics::d3dx
