#include "nfs/graphics/backend/eagl_backend.hpp"
namespace nfs::graphics::backend {
Presentation presentation{};Dword desktop_mode[4]{};Dword material_color=0;  
Dword stream_state=4294967295u,depth_write_state=4294967295u;std::uint8_t window_bound=0;MSG message{};WORD saved_gamma[3][256]{};  

void set_dimensions(Dword* object,float width,float height){object[8]=static_cast<Dword>(static_cast<int>(width));object[9]=static_cast<Dword>(static_cast<int>(height));}

void get_dimensions(Dword* object,float* width,float* height){*width=static_cast<float>(object[8]);*height=static_cast<float>(object[9]);}

void set_color_depth(Dword* object,Dword value){object[40/4]=object[44/4]=value;}

Dword color_depth(Dword* object){return object[44/4];}

void set_depth_bits(Dword* object,Dword value){object[52/4]=value;}

Dword requested_colour_depth(Dword* object){return object[40/4];}

void set_display_mode(Dword* object,Dword value){Dword* c=owner(object);if(c[152/4]!=value)c[156/4]=c[152/4];owner(object)[152/4]=value;}
// Division traps and signed overflow remain outside accepted inputs.
int advance_frame(Dword* object){int divisor=static_cast<int>(object[48/4]+1),value=static_cast<int>(object[128/4]+1);object[128/4]=value%divisor;return value/divisor;}

void release_backbuffer(Dword* object){Dword* c=owner(object);if(c[200/4]){com(reinterpret_cast<Dword*>(c[200/4]),8);owner(object)[200/4]=0;}}

std::uint8_t supports_format(Dword* object,Dword adapter,int colour,int depth,int stencil,std::uint8_t windowed){if(!backend_interface)return 0;Dword format=choose_backbuffer(object,adapter,colour,windowed);return format&&exact_depth(object,adapter,format,depth,stencil);}
}
