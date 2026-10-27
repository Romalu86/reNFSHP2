#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstring>
namespace nfs::graphics::textures {
// Zero is accepted; the high-bit singleton is excluded.
bool is_power(Dword value){return !(value&(value-1))&&value!=2147483648u;}
// An exact power above 2 is halved, not left unchanged.
void lower_power(Dword* value){Dword input=*value;if(input>2){Dword bit=2147483648u;int index=31;while(!(bit&input)&&index>1){bit>>=1;--index;}
 if(index!=1)*value=input==bit?input>>1:bit;}}

void constrain_dimensions(const TextureReader* object,Dword* width,Dword* height){const auto* b=reinterpret_cast<const std::uint8_t*>(object);
 if(!b[12]){if(!b[13])return;}else if(!b[13]){if(!is_power(*width))lower_power(width);if(!is_power(*height))lower_power(height);return;}
 Dword value=*height<=*width?*height:*width;if(b[12]||!b[13])lower_power(&value);*width=*height=value;}
// Independent maxima, power correction, and square correction retain their order.
void clamp_dimensions(const TextureReader* object,Dword* width,Dword* height){Dword max_width=object->fields[4],max_height=object->fields[5];
 const auto* b=reinterpret_cast<const std::uint8_t*>(object);if((b[12]&&(!is_power(max_width)||!is_power(max_height)))||(b[13]&&max_width!=max_height)){
  constrain_dimensions(object,&max_width,&max_height);}
 if(max_width<*width)*width=max_width;if(max_height<*height)*height=max_height;
 if(b[12]){if(!is_power(*width))lower_power(width);if(!is_power(*height))lower_power(height);}if(b[13]&&*width!=*height){Dword minimum=*height<=*width?*height:*width;*width=*height=minimum;}}

bool outside_range(Dword begin,Dword value,Dword extent){return value<begin||extent<value-begin;}
// Indexed fallback has its own table and is not selected here.
Dword fallback_format(Dword bits,Dword alpha){Fallback* selected=(bits==4||bits==8||bits==15||bits==16)?packed_fallback:colour_fallback;
 for(int i=0;i<3;++i)if(selected[i].alpha==alpha)return selected[i].format;
 for(const auto& row:packed_fallback)if(row.alpha==alpha)return row.format;
 for(const auto& row:colour_fallback)if(row.alpha==alpha)return row.format;return 0;}
// Arithmetic before conversion wraps to a 32-bit unsigned word.
double storage_bytes(Dword format,int width,int height){Dword bits=32;for(const auto& row:formats)if(row.format==format){bits=row.bits;break;}
 return static_cast<double>((((bits>>3)*static_cast<Dword>(width)+15)&~15u)*static_cast<Dword>(height));}
double storage_ratio(const Dword* image,Dword format){const Dword depth=tar::image_depth(image);
 Dword bits=32;for(const auto& row:formats)if(row.format==format){bits=row.bits;break;}
 return static_cast<double>(bits)/depth;}
// The CRT locale controls character case conversion.
bool extension(const char* path,const char* suffix){if(!path)return false;const char* dot=std::strrchr(path,'.');return dot&&!_stricmp(dot,suffix);}
bool is_fsh(const char* path){return extension(path,".FSH");}bool is_viv(const char* path){return extension(path,".VIV");}
}
