#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::backend::unresolved {
extern Dword vertex_shader,vertex_values,vertex_extra,vertex_parameters[3];
}
namespace nfs::graphics::backend {
using namespace unresolved;
#include "eagl_primitive_counts.inc"

void set_vertex_handle(Dword value){if(vertex_values!=value){vertex_values=value;com(backend_device,304,value);selected_device();}}

void vertex_constants(Dword first,Dword data,Dword count){com(backend_device,316,first,data,count);}

void bind_stream(Dword stage,Dword* buffer){if(resources::bound_streams[stage]!=reinterpret_cast<Dword>(buffer)){com(backend_device,332,stage,buffer[1],buffer[4]);resources::bound_streams[stage]=reinterpret_cast<Dword>(buffer);}}

void index_buffer(Dword* buffer){resources::bound_index=reinterpret_cast<Dword>(buffer);}

void index_base(Dword value){vertex_extra=value;}

void bind_indices(){if(vertex_parameters[0]!=resources::bound_index||vertex_parameters[1]!=vertex_extra){vertex_parameters[0]=resources::bound_index;vertex_parameters[1]=vertex_extra;com(backend_device,340,*reinterpret_cast<Dword*>(resources::bound_index),vertex_extra);}}

void pixel_constants(Dword first,Dword data,Dword count){com(backend_device,364,first,data,count);}

void draw_primitives(Dword first,Dword,Dword count){com(backend_device,280,geoprim::current_primitive,first,count);}

void draw_indexed(Dword first,Dword vertices,Dword index,Dword,Dword count){com(backend_device,284,geoprim::current_primitive,first,vertices,index,count);}

void draw_user(Dword vertices,Dword data,Dword stride_value){if(com(backend_device,12)==static_cast<int>(2289436776u))return;Dword kind=geoprim::current_primitive;int count=static_cast<int>(vertices-primitive_counts[kind][1])/static_cast<int>(primitive_counts[kind][0]);com(backend_device,288,kind,count,data,stride_value);resources::bound_index=vertex_extra=resources::bound_streams[0]=0;}

void draw_indexed_user(Dword first,Dword vertices,Dword data,Dword stride_value,Dword indices,Dword index_data){if(com(backend_device,12)==static_cast<int>(2289436776u))return;Dword kind=geoprim::current_primitive;int count=static_cast<int>(indices-primitive_counts[kind][1])/static_cast<int>(primitive_counts[kind][0]);com(backend_device,292,kind,first,vertices,count,index_data,Dword(101),data,stride_value);resources::bound_index=vertex_extra=resources::bound_streams[0]=0;vertex_parameters[0]=vertex_parameters[1]=~0u;}

void clear_index_bindings(){resources::bound_index=vertex_extra=0;vertex_parameters[0]=vertex_parameters[1]=~0u;}

void clear_stream_bindings(){for(Dword& stream:resources::bound_streams)stream=0;}

Dword bound_pixel_shader(){return vertex_shader;}

void texture_factor(Dword value){if(vertex_parameters[2]!=value){vertex_parameters[2]=value;com(backend_device,200,Dword(60),value);}}

TextureStageState* construct_texture_stages(TextureStageState* object){for(int stage=0;stage<8;++stage){Dword values[17]={Dword(stage==0?4:1),1,2,1,Dword(stage==0?2:1),1,2,1,1,0,0,0,0,Dword(stage),0,0,0};for(int row=0;row<17;++row)object->values[row][stage]=values[row];}object->tail=0;return object;}
// The destructor consists of RET and does not read this.
void destroy_texture_stages(TextureStageState*){}
}
