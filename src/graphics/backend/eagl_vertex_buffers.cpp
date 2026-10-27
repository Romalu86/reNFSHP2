#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::graphics::buffers {
const Dword vertex_methods[3]={reinterpret_cast<Dword>(vertex_delete),reinterpret_cast<Dword>(vertex_lost),reinterpret_cast<Dword>(vertex_restore)};

Dword* vertex_construct(Dword* object,Dword bytes,Dword stride_value,Dword usage_value,Dword selection,Dword vertex_layout,Dword reserved){object[3]=bytes;object[7]=usage_value;object[0]=reinterpret_cast<Dword>(vertex_methods);object[2]=0;object[4]=stride_value;object[5]=reserved;object[6]=0;bool capability;if(!selection){object[8]=0;capability=state::supports_hardware_vertex_programs()!=0;}else{object[8]=vertex_layout;capability=state::supports_hardware_fixed_function_processing()!=0;}if(!capability)object[7]|=16;backend::com(backend_device,92,object[3],object[7],object[8],Dword(0),object+1);return object;}

void vertex_destroy(Dword* object){object[0]=reinterpret_cast<Dword>(vertex_methods);if(object[1])backend::com(reinterpret_cast<Dword*>(object[1]),8);object[1]=object[6]=0;}

void* __fastcall vertex_delete(Dword* object,void*,unsigned flags){vertex_destroy(object);if(flags&1)eagl::deallocator(object,36);return object;}

Dword used_bytes(Dword* object){return object[2];}

Dword capacity(Dword* object){return object[3];}

Dword usage(Dword* object){return object[7];}

Dword layout(Dword* object){return object[8];}

Dword stride(Dword* object){return object[4];}

Dword data(Dword* object){return object[6];}

void replace_data(Dword* object,Dword source,Dword bytes){Dword mapped;lock(object,0,bytes,&mapped,8192);kernel::memory_copy(reinterpret_cast<void*>(mapped),reinterpret_cast<void*>(source),bytes);unlock(object);object[2]=bytes;}

Dword lock(Dword* object,Dword offset,Dword bytes,Dword* output,Dword flags){return backend::com(reinterpret_cast<Dword*>(object[1]),44,offset,bytes,output,flags);}

Dword unlock(Dword* object){return backend::com(reinterpret_cast<Dword*>(object[1]),48);}

void __fastcall vertex_lost(Dword* object,void*){if(object[1]){backend::com(reinterpret_cast<Dword*>(object[1]),8);object[1]=0;}}
// Entry is the saved vtable target, not adjacent junk bytes.
void __fastcall vertex_restore(Dword* object,void*){if(!state::supports_hardware_vertex_programs()&&!state::supports_hardware_fixed_function_processing())object[7]|=16;backend::com(backend_device,92,object[3],object[7],object[8],Dword(0),object+1);if(object[6]){Dword mapped;lock(object,0,object[3],&mapped,0);kernel::memory_copy(reinterpret_cast<void*>(mapped),reinterpret_cast<void*>(object[6]),object[3]);unlock(object);}}
static void register_vertex(Dword* object){for(int i=0;i<resources::vertex_capacity;++i)if(!resources::cache.vertices[i]){resources::cache.vertices[i]=reinterpret_cast<Dword>(object);++resources::cache.vertex_count;break;}}

Dword* create_vertex(Dword bytes,Dword stride_value,Dword usage_value,Dword vertex_layout,Dword selection,Dword reserve){if(!reserve)reserve=bytes;auto* object=static_cast<Dword*>(eagl::allocator(36,"EAGL::VertexBuffer new"));if(object)vertex_construct(object,reserve,stride_value,usage_value,selection,vertex_layout,0);register_vertex(object);return object;}

Dword* create_vertex_data(Dword bytes,Dword stride_value,Dword usage_value,Dword vertex_layout,Dword source,Dword selection){auto* object=static_cast<Dword*>(eagl::allocator(36,"EAGL::VertexBuffer new"));if(object)vertex_construct(object,bytes,stride_value,usage_value,selection,vertex_layout,0);Dword mapped;lock(object,0,bytes,&mapped,0);kernel::memory_copy(reinterpret_cast<void*>(mapped),reinterpret_cast<void*>(source),bytes);unlock(object);object[6]=source;object[2]+=bytes;register_vertex(object);return object;}

Dword* create_index_data(Dword bytes,Dword source,Dword* offset,Dword selection){auto* object=static_cast<Dword*>(eagl::allocator(20,"EAGL::IndexBuffer new"));if(object)resources::index_construct(object,bytes,selection);Dword mapped;resources::index_lock(object,0,bytes,&mapped,0);kernel::memory_copy(reinterpret_cast<void*>(mapped),reinterpret_cast<void*>(source),bytes);resources::index_unlock(object);object[4]=source;for(int i=0;i<resources::index_capacity;++i)if(!resources::cache.indices[i]){resources::cache.indices[i]=reinterpret_cast<Dword>(object);++resources::cache.index_count;break;}object[1]+=bytes;*offset=0;return object;}
}
