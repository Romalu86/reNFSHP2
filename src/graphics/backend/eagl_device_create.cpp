#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstring>
namespace nfs::graphics::backend {
// Software vertex-processing retry is attempted after any failed first CreateDevice.
int create_hal_device(Dword* object,std::uint8_t probing){invalidate_pixel_shader_cache();if(!render_window)return static_cast<int>(2147500037u);presentation.flags=owner(object)[172/4];
    auto depth=[&]{Dword* c=owner(object);if(c[52/4]){c[164/4]=1;c=owner(object);choose_depth(object,c[140/4],1,reinterpret_cast<int*>(c+52/4),reinterpret_cast<int*>(c+48/4),c+5);presentation.depth_enabled=1;presentation.depth_format=owner(object)[5];}else presentation.depth_enabled=0;};
    depth();owner(object)[164/4]=1;object[0]=com(backend_interface,60,owner(object)[140/4],Dword(1),render_window,Dword(128),&presentation,&backend_device);
    if(static_cast<int>(object[0])<0){depth();owner(object)[164/4]=1;object[0]=com(backend_interface,60,owner(object)[140/4],Dword(1),render_window,Dword(32),&presentation,&backend_device);if(static_cast<int>(object[0])<0)return 0;}
    com(backend_device,64,Dword(0),Dword(0),owner(object)+200/4);if(presentation.depth_enabled)com(backend_device,132,owner(object)+204/4);
    owner(object)[40/4]=format_depth(owner(object)[4]);owner(object)[44/4]=owner(object)[40/4];char error[256];Dword selected=owner(object)[152/4];
    if(selected==0||selected==2)hardware_caps=retrieve_capabilities(backend_device,error);else if(selected==1||selected==3)alternate_caps=retrieve_capabilities(backend_device,error);
    if(!probing){detect_vertex_programs(object);detect_fixed_function_vertex_processing(object);}return static_cast<int>(object[0]);}
// The external-window path deliberately leaves presentation.window zero.
std::uint8_t external_window(Dword* object,std::uint8_t windowed,std::uint8_t probing){Dword* c=owner(object);if(!c[160/4])return 0;
    if(c[156/4]!=c[152/4]){c[156/4]=c[152/4];if(render_window&&owner(object)[156/4]!=3&&owner(object)[156/4]!=2)DestroyWindow(render_window);}
    render_window=*reinterpret_cast<HWND*>(owner(object)[160/4]);presentation={};if(windowed)windowed_formats(object,probing);else fullscreen_formats(object,probing);
    c=owner(object);presentation.count=c[56/4];presentation.width=c[8];presentation.height=c[9];if(c[7]<=3)presentation.swap=c[7]+1;presentation.multisample=c[6];
    int result=backend_device?reset_device(object):create_hal_device(object,probing);return result>=0&&backend_device;}
}
