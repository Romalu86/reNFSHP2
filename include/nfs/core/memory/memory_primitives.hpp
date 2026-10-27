#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::kernel {
extern Dword cpu_mmx, cpu_stream_mmx, cpu_sse;
void *__cdecl memory_copy(void *, const void *, Dword);
void *__cdecl memory_zero(void *, Dword);
void *__cdecl memory_fill(void *, Dword, Dword);
void __cdecl memory_move(void *, const void *, Dword);
// Direct MMX variants retain the native MMX state. The dispatchers execute EMMS
// after their MMX branch; SSE variants retain the native non-temporal stores.
void *__cdecl memory_copy_scalar(void *, const void *, Dword);
void *__cdecl memory_copy_mmx(void *, const void *, Dword);
void *__cdecl memory_copy_stream_mmx(void *, const void *, Dword);
void *__cdecl memory_copy_sse(void *, const void *, Dword);
void *__cdecl memory_fill_scalar(void *, Dword, Dword);
void *__cdecl memory_fill_mmx(void *, Dword, Dword);
void *__cdecl memory_fill_stream_mmx(void *, Dword, Dword);
void *__cdecl memory_fill_sse(void *, Dword, Dword);
} // namespace nfs::kernel
