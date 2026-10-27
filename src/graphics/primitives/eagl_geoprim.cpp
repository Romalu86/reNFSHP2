#include "nfs/graphics/resources/eagl_assets.hpp"
#include <initializer_list>
namespace nfs::graphics::geoprim {
// Complete body returns this.
Dword* base_construct(Dword* object){return object;}
 
void base_destroy(Dword*){}

void destroy(Dword* object){base_destroy(object);}
static void copy_fields(Dword* output,const Dword* input){for(unsigned i:{0,1,5,10,2,3,4,6,7,8,9,13,14,12,11,15,16})output[i]=input[i];}

Dword* copy_construct(Dword* object,const Dword* input){base_construct(object);copy_fields(object,input);return object;}

Dword* assign(Dword* object,const Dword* input){copy_fields(object,input);return object;}

std::uint8_t primitive(Dword* object,Dword value){object[0]=value;return 1;}

std::uint8_t shading(Dword* object,Dword value){object[1]=value;return 1;}

std::uint8_t culling(Dword* object,Dword value){object[2]=value&255;return 1;}

std::uint8_t depth_method(Dword* object,Dword value){object[4]=value;return 1;}

std::uint8_t read_depth_method(Dword* object,Dword* output){*output=object[4];return 1;}

std::uint8_t alpha_blend_mode(Dword* object,Dword value){object[5]=value;switch(value){case 1:object[13]=5;object[14]=6;break;case 2:object[13]=5;object[14]=2;break;case 3:object[13]=1;object[14]=6;break;case 4:object[13]=9;object[14]=1;break;case 5:object[13]=2;object[14]=2;break;default:break;}return 1;}

std::uint8_t alpha_test(Dword* object,Dword value){object[6]=value&255;return 1;}

std::uint8_t alpha_reference(Dword* object,Dword value){object[7]=value;return 1;}

std::uint8_t alpha_method(Dword* object,Dword value){object[8]=value;return 1;}

std::uint8_t texture_enable(Dword* object,Dword value){object[9]=value&255;return 1;}
// Returns zero without reading the receiver or argument.
std::uint8_t texture_coordinates(Dword*,Dword){return 0;}

std::uint8_t transparency(Dword* object,Dword value){object[10]=value;return 1;}
// Complete body returns zero.
std::uint8_t chroma_colour(Dword*,Dword){return 0;}

std::uint8_t cull_direction(Dword* object,Dword value){object[3]=value;return 1;}

std::uint8_t patch_segments(Dword* object,Dword value){object[15]=value;return 1;}

std::uint8_t fill_mode(Dword* object,Dword value){object[11]=value;return 1;}

std::uint8_t blend_operation(Dword* object,Dword value){object[12]=value;return 1;}

std::uint8_t blend_factors(Dword* object,Dword source,Dword destination,Dword operation){object[13]=source;object[12]=operation;object[14]=destination;return 1;}

std::uint8_t depth_write(Dword* object,Dword value){object[16]=value&255;return 1;}

void runtime_release(Dword* object,Dword){if(object){destroy(object);eagl::deallocator(object,68);}}
}
