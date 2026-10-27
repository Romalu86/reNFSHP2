#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::textures {

Dword shape_type(const Dword* image){return *image&127;}
std::uint8_t palette_alpha(const std::uint8_t* palette){std::uint8_t result=0;for(int i=0;i<256&&result!=3;++i){auto value=palette[i*4+3];if(!value)result=3;if(value&&value!=255)result|=2;}return result&254;}
static const std::uint8_t* pixels(const Dword* image){return reinterpret_cast<const std::uint8_t*>((image[3]&4096)?reinterpret_cast<Dword>(image)+image[4]:reinterpret_cast<Dword>(image+4));}

void alpha1(const Dword* image,Dword* result){Dword pitch=shape::pitch(image),width=static_cast<std::int16_t>(image[1]),height=static_cast<std::int16_t>(image[1]>>16);int stride=tar::image_depth(image)>>3;
 const auto* row=pixels(image);*result=0;for(Dword y=0;y<height;++y,row+=pitch){const auto* p=row;for(Dword x=0;x<width;++x,p+=stride)if(!(p[1]&128)){*result=1;return;}}}
void alpha4(const Dword* image,Dword* result){Dword pitch=shape::pitch(image),width=static_cast<std::int16_t>(image[1]),height=static_cast<std::int16_t>(image[1]>>16);int stride=tar::image_depth(image)>>3;
 const auto* row=pixels(image);*result=0;for(Dword y=0;y<height;++y,row+=pitch){const auto* p=row;for(Dword x=0;x<width;++x,p+=stride){Dword value=*reinterpret_cast<const std::uint16_t*>(p)&61440;
  if(!value)*result=1;else if(value!=61440){*result=2;return;}else if(*result>1)return;}}}
void alpha8(const Dword* image,Dword* result){Dword pitch=shape::pitch(image),width=static_cast<std::int16_t>(image[1]),height=static_cast<std::int16_t>(image[1]>>16);int stride=tar::image_depth(image)>>3;
 const auto* row=pixels(image);*result=0;for(Dword y=0;y<height;++y,row+=pitch){const auto* p=row;for(Dword x=0;x<width;++x,p+=stride){Dword value=*reinterpret_cast<const Dword*>(p)&4278190080u;
  if(!value)*result=1;else if(value!=4278190080u){*result=2;return;}else if(*result>1)return;}}}
// DXT1 is classified as binary alpha without scanning it.
std::uint8_t alpha(const Dword* image,std::uint8_t scan){Dword type=*image&127;if(type==96)return 1;if(type==97||type==98)return 2;
 if(auto* palette=shape::palette(const_cast<Dword*>(image)))return palette_alpha(pixels(palette));
 int bits=tar::image_depth(image);shape::pitch(image);Dword result=0;
 if(bits==15){if((*image&255)==126){if(scan)alpha1(image,&result);else result=1;}}
 else if(bits==16){if((*image&255)==109){if(scan)alpha4(image,&result);else result=2;}}
 else if(bits==32){if(scan)alpha8(image,&result);else result=2;}return result==0?0:result==1?1:2;}
// The largest chunk address and the final chunk's size are used separately.
Dword* shape_end(Dword* image){if(!image)return nullptr;Dword* last=image;while(static_cast<int>(*image)>>8){auto* next=reinterpret_cast<Dword*>(reinterpret_cast<Dword>(image)+(static_cast<int>(*image)>>8));
 if(!next)break;image=next;if(last<next)last=next;}return reinterpret_cast<Dword*>(reinterpret_cast<Dword>(last)+shape::size(image));}
}
