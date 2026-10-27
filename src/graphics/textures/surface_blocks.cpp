#include "nfs/graphics/textures/surface_conversion.hpp"
#include <cstring>
namespace nfs::graphics::surfaces {
// A compressed copy requires all four XY bounds divisible by 4.
int copy_blocks(Transfer* object){Dword* source=object->source;Dword* destination=object->destination;
 if((source[1036]|source[1037]|source[1038]|source[1039]|destination[1036]|destination[1037]|destination[1038]|destination[1039])&3)return static_cast<int>(2147500037u);
 Dword block;if(destination[1]==827611204)block=8;else if(destination[1]==844388420||destination[1]==861165636||destination[1]==877942852||destination[1]==894720068)block=16;else return static_cast<int>(2147500037u);
 auto* output=reinterpret_cast<std::uint8_t*>(destination[6]+(destination[1037]>>2)*destination[1044]+destination[1045]*destination[1040]+(destination[1036]>>2)*block);
 auto* input=reinterpret_cast<const std::uint8_t*>(source[6]+(source[1037]>>2)*source[1044]+source[1045]*source[1040]+(source[1036]>>2)*block);
 for(Dword z=0;z<destination[1048];++z){auto* out=output;auto* in=input;for(Dword y=0;y<destination[1047];y+=4){copy_forward(out,in,(destination[1046]>>2)*block);out+=destination[1044];in+=source[1044];}output+=destination[1045];input+=source[1045];}return 0;}
}
