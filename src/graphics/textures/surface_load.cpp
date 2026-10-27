#include "nfs/graphics/textures/surface_conversion.hpp"
#include <cstring>
namespace nfs::graphics::surfaces {
// This body is linked by TAR; unsupported codec/filter
// implementations remain unresolved symbols, never success-returning fallbacks.
int __stdcall load(Dword* destination,Dword destination_palette,const RECT* destination_rect,Dword* source,Dword format,Dword pitch,const void* source_palette,const RECT* source_rect,Dword filter,Dword key){
 Transfer operation;construct(&operation);SurfaceMap mapping;construct(&mapping);int result;
 if(!destination||!source||!source_rect)result=static_cast<int>(2289436780u);
 else{if(filter==~0u)filter=524292;Image mapped;result=map(&mapping,&mapped,destination,destination_palette,destination_rect,0,0);
  if(result>=0){Image input;input.words[0]=reinterpret_cast<Dword>(source);input.words[1]=format;input.words[2]=pitch;input.words[3]=0;
   std::memcpy(input.words+4,source_rect,16);input.words[8]=0;input.words[9]=1;std::memcpy(input.words+10,input.words+4,24);
   // The word at byte offset 64 remains uninitialized.
    
   input.words[17]=key;input.words[18]=reinterpret_cast<Dword>(source_palette);result=transfer(&operation,&mapped,&input,filter);if(result>=0)result=0;}}
 destroy(&mapping);destroy(&operation);return result;}
}
