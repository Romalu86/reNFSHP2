#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::graphics::methods {
#include "../loader/eagl_generation_messages.inc"

Dword* duplicate_coordinates(int copies,Dword layout_value,int stride_value,int bytes,Dword source){if(copies<0)copies=1;int vertices=bytes/stride_value;Dword size=static_cast<Dword>(vertices)*copies+1;Dword element=(layout_value&16)?32:16;size*=element;auto* storage=static_cast<Dword*>(eagl::allocator(size,(layout_value&16)?normal_label:coordinate_label));if(!storage)return nullptr;*storage=size;Dword* output=storage+4;Dword destination=reinterpret_cast<Dword>(output);for(int i=0;i<copies;++i){Dword input=source;Dword write=destination;for(int j=0;j<vertices;++j){kernel::memory_copy(reinterpret_cast<void*>(write),reinterpret_cast<void*>(input),element);write+=element;input+=stride_value;}destination+=vertices*element;}return output;}

Dword* clone_vertex(Dword* source,std::uint8_t selection,std::uint8_t skinned,Dword copies,std::uint8_t software){Dword flags=buffers::usage(source),stride_value=buffers::stride(source),bytes=buffers::used_bytes(source),layout_value=buffers::layout(source);bool capability=selection?state::supports_hardware_fixed_function_processing()!=0:state::supports_hardware_vertex_programs()!=0;if((!capability&&flags==0)||(capability&&flags!=0)){Dword data=buffers::data(source);flags=capability?(flags&~16u):0;if(!skinned)return buffers::create_vertex_data(bytes,stride_value,flags,layout_value,data,selection);return buffers::create_skinned(static_cast<int>(copies),bytes,stride_value,flags,layout_value,data,selection,software);}return nullptr;}
}
