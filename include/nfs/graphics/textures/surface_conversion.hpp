#pragma once
#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstring>
namespace nfs::graphics::surfaces {
// REP MOVSD / MOVSB order, including overlapping source/destination.
inline void copy_forward(void* destination,const void* source,Dword bytes){auto* out=static_cast<std::uint8_t*>(destination);auto* in=static_cast<const std::uint8_t*>(source);
 while(bytes>=4){Dword value;std::memcpy(&value,in,4);std::memcpy(out,&value,4);in+=4;out+=4;bytes-=4;}while(bytes--)*out++=*in++;}
struct Image {Dword words[19];};
struct SurfaceMap {Dword flags;Dword* surface;Dword* temporary;Dword* device;};
struct VolumeMap {Dword* volume;};
struct Transfer {Dword* source;Dword* destination;Dword filter;};
static_assert(sizeof(Image)==76&&sizeof(SurfaceMap)==16&&sizeof(Transfer)==12);
using DebugMute=void(__cdecl*)(Dword);
extern DebugMute mute_runtime,mute_debug;
void debug_mute(Dword);SurfaceMap* construct(SurfaceMap*);int unlock(SurfaceMap*);int destroy(SurfaceMap*);
VolumeMap* construct(VolumeMap*);int unlock(VolumeMap*);int destroy(VolumeMap*);
int map(SurfaceMap*,Image*,Dword*,Dword,const RECT*,Dword,Dword);
Transfer* construct(Transfer*);void destroy(Transfer*);int transfer(Transfer*,Image*,Image*,Dword);
int copy(Transfer*);int copy_blocks(Transfer*);int convert_rows(Transfer*);int clip(Transfer*);int point(Transfer*);
int __stdcall load(Dword*,Dword,const RECT*,Dword*,Dword,Dword,const void*,const RECT*,Dword,Dword);

}
