#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::backend::unresolved {extern Dword vertex_shader,vertex_values;}
namespace nfs::graphics::resources {

Dword* vertex_program_construct(Dword* object,Dword declaration,Dword code){object[1]=declaration;object[2]=code;reinterpret_cast<std::uint8_t*>(object)[12]=0;object[4]=1;bool hardware=state::supports_hardware_vertex_programs()!=0;backend::set_software_processing(hardware?0:1);backend::com(backend_device,300,declaration,code,object,Dword(hardware?0:16));for(Dword* item=cache.vertex_head;item;item=reinterpret_cast<Dword*>(item[5]))if(*object==*item){object[4]=item[4];++item[4];}reinterpret_cast<std::uint8_t*>(object)[12]=0;object[5]=reinterpret_cast<Dword>(cache.vertex_head);object[6]=0;if(cache.vertex_head)cache.vertex_head[6]=reinterpret_cast<Dword>(object);cache.vertex_head=object;return object;}

void bind_vertex(Dword* object){auto& value=backend::unresolved::vertex_values;if(value!=*object){value=*object;backend::com(backend_device,304,*object);backend::selected_device();}}

void bind_pixel(Dword* object){backend::bind_pixel_shader(*object);}
}
