#include "nfs/input/window/window_input.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstring>
namespace nfs::graphics::backend {

std::uint8_t set_table_fog(Dword* object,Dword value){if(backend_device&&(adapter_caps()[584/4]&256)!=256)return 0;owner(object)[112/4]=value;return 1;}

std::uint8_t set_vertex_fog(Dword* object,Dword value){if(backend_device&&(adapter_caps()[584/4]&128)!=128)return 0;owner(object)[116/4]=value;return 1;}
// Initialize device state and its dependencies.
std::uint8_t initialize_context(Dword* object){using namespace unresolved;std::uint8_t result=1;switch(object[152/4]){case 0:result=fullscreen_window(object);break;case 1:result=windowed_window(object);break;case 2:result=external_window(object,0,0);break;case 3:result=external_window(object,1,0);break;default:break;}if(!result)return 0;
    for(int i=0;i<8;++i)texture_cache[i]=0;Dword* caps=adapter_caps();display_mode=object[152/4];com(backend_device,76,saved_gamma);
    auto render=[&](Dword name,Dword value){com(backend_device,200,name,value);};auto* bytes=reinterpret_cast<std::uint8_t*>(object);
    render(26,bytes[68]);render(14,bytes[109]);render(22,object[64/4]);render(54,object[76/4]);render(55,object[80/4]);render(53,object[84/4]);render(56,object[88/4]);render(57,object[92/4]);render(58,object[96/4]);render(59,object[100/4]);render(52,bytes[108]);render(27,1);render(7,object[72/4]);render(28,fog_enabled);
    if((caps[584/4]&256)==256)render(35,table_fog);if((caps[584/4]&128)==128)render(140,vertex_fog);
    render(36,object[120/4]);render(37,object[124/4]);render(38,object[128/4]);render(34,4294967295u);render(143,1);
    int lights=static_cast<int>(caps[708/4]);for(int i=0;i<lights&&i<16;++i)com(backend_device,184,i,Dword(i<3?1:0));object[144/4]=lights;
    Dword factor;if(lights<=0)factor=1065353216;else if(lights>=3)factor=1051372191;else{float fraction=1.0f/lights;std::memcpy(&factor,&fraction,4);}render(137,lighting_enabled);material_color=factor;
    Dword material[17]{};material[4]=material[5]=material[6]=material[7]=material_color;com(backend_device,168,material);render_width=object[8];render_height=object[9];com(backend_device,144,Dword(0),Dword(0),Dword(3),Dword(4278190080u),1.0f,Dword(0));Sleep(0);
    if(com(backend_device,60,Dword(0),Dword(0),Dword(0),Dword(0))==static_cast<int>(2289436776u)&&!recover_device(object))return 0;
    if(bytes[168]){bytes[168]=0;for(Dword* method=resources::render_methods;method;method=reinterpret_cast<Dword*>(method[4]))methods::initialize(method);for(Dword* method=secondary_methods;method;method=reinterpret_cast<Dword*>(method[4]))methods::initialize_variant(method);}
    if(window_bound)window_input::bound_cursor(0,0,object[8],object[9]);object[136/4]=1;Dword indices=index_capacity(selected_device());Dword vertices=vertex_capacity(selected_device());resources::initialize(vertices,indices);
    buffer_bindings();render_bindings();vertex_state();initialize_texture_stages();texture_state();stream_state=4294967295u;state::render_frame_serial=0;return 1;}}
