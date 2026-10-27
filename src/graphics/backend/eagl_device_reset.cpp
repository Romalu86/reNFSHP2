#include "nfs/graphics/backend/eagl_backend.hpp"
namespace nfs::movies { void release_movie_device_textures(); }
namespace nfs::graphics::backend {

std::uint8_t set_software_processing(Dword value){if(stream_state!=value){unresolved::vertex_state();stream_state=value;com(backend_device,200,Dword(153),value);}return 1;}

void vertex_programs_restored(){for(Dword* item=resources::cache.vertex_head;item;item=reinterpret_cast<Dword*>(item[5]))if(!reinterpret_cast<std::uint8_t*>(item)[12]&&item[1]&&item[2]){
    Dword usage;if(!state::supports_hardware_vertex_programs()){set_software_processing(1);usage=16;}else{set_software_processing(0);usage=0;}com(backend_device,300,item[1],item[2],item,usage);++item[4];}}
 
int reset_device(Dword* object){invalidate_pixel_shader_cache();movies::release_movie_device_textures();resources::device_lost();release_backbuffer(object);Dword* c=owner(object);if(c[204/4]){com(reinterpret_cast<Dword*>(c[204/4]),8);owner(object)[204/4]=0;}
    unresolved::buffers_lost();targets_lost();depth_targets_lost();resources::vertex_programs_lost();pixel_programs_lost();
    auto prepare=[&]{presentation.flags=owner(object)[172/4];if(presentation.windowed)windowed_formats(object,0);else fullscreen_formats(object,0);
        if(presentation.windowed)owner(object)[52/4]=format_depth(presentation.format);Dword* render_context=owner(object);
        if(!render_context[52/4])presentation.depth_enabled=0;else{if(!choose_depth(object,render_context[140/4],render_context[164/4],reinterpret_cast<int*>(render_context+52/4),reinterpret_cast<int*>(render_context+48/4),render_context+5))return false;
            presentation.depth_enabled=1;presentation.depth_format=owner(object)[5];}return true;};
    if(!prepare())return static_cast<int>(2147500037u);int result=com(backend_device,56,&presentation);
    if(result<0){ShowWindow(render_window,6);ShowWindow(render_window,9);SetActiveWindow(render_window);Sleep(1);if(!prepare())return static_cast<int>(2147500037u);result=com(backend_device,56,&presentation);if(result<0)return result;}
    SetFocus(render_window);ShowWindow(render_window,9);UpdateWindow(render_window);SetCursor(nullptr);char error[256];Dword selected=owner(object)[152/4];
    if(selected==0||selected==2)hardware_caps=retrieve_capabilities(backend_device,error);else if(selected==1||selected==3)alternate_caps=retrieve_capabilities(backend_device,error);
    detect_vertex_programs(object);detect_fixed_function_vertex_processing(object);resources::device_restored();com(backend_device,64,Dword(0),Dword(0),owner(object)+200/4);
    if(presentation.depth_enabled)com(backend_device,132,owner(object)+204/4);vertex_programs_restored();pixel_programs_restored();unresolved::buffers_restored();targets_restored();depth_targets_restored();
    com(backend_device,200,Dword(137),Dword(unresolved::lighting_enabled));int lights=static_cast<int>(adapter_caps()[708/4]);
    for(int i=0;i<lights&&i<16;++i)com(backend_device,184,i,Dword(i<3?1:0));Dword material[17]{};material[4]=material[5]=material[6]=material[7]=material_color;com(backend_device,168,material);return 0;}}
