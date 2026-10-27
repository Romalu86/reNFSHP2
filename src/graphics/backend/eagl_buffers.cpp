#include "nfs/graphics/resources/eagl_cleanup.hpp"
#include "nfs/graphics/backend/eagl_state.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::graphics::resources {
static Dword* pointer(Dword value){return reinterpret_cast<Dword*>(value);}
static Dword* allocate_clear(Dword bytes,const char* label){auto* result=static_cast<Dword*>(eagl::allocator(bytes,label));for(Dword i=0;i<(bytes>>2);++i)result[i]=0;return result;}

void initialize(Dword vertices,Dword indices){if(!cache.ready){vertex_capacity=vertices;index_capacity=indices;
    cache.uses=allocate_clear(vertices*20,"EAGL::VertexBufferType new[]");cache.vertices=allocate_clear(vertices*4,"EAGL::BufferManager::mVertexBufferArray");
    cache.indices=allocate_clear(indices*4,"EAGL::BufferManager::mIndexBufferArray");cache.vertex_free_positions=allocate_clear(vertices*4,"EAGL::BufferManager::mVertexBufferFreePositionArray");
    cache.index_free_positions=allocate_clear(indices*4,"EAGL::BufferManager::mIndexBufferFreePositionArray");cache.ready=1;}}
static void create_index(Dword* object){using Create=Dword(__stdcall*)(Dword*,Dword,Dword,Dword,Dword,Dword*);reinterpret_cast<const Create*>(backend_device[0])[96/4](backend_device,object[2],object[3],101,0,object);}

Dword* index_construct(Dword* object,Dword bytes,Dword selection){object[2]=bytes;object[1]=0;object[4]=0;bool capability=selection?state::supports_hardware_fixed_function_processing()!=0:state::supports_hardware_vertex_programs()!=0;object[3]=capability?8:24;create_index(object);return object;}

Dword* index_construct_usage(Dword* object,Dword bytes,Dword usage,Dword selection){object[2]=bytes;object[1]=0;object[4]=0;bool capability=selection?state::supports_hardware_fixed_function_processing()!=0:state::supports_hardware_vertex_programs()!=0;object[3]=capability?usage:usage|16;create_index(object);return object;}

Dword index_lock(Dword* object,Dword offset,Dword bytes,Dword* output,Dword flags){Dword* resource=pointer(object[0]);using Lock=Dword(__stdcall*)(Dword*,Dword,Dword,Dword*,Dword);return reinterpret_cast<const Lock*>(resource[0])[44/4](resource,offset,bytes,output,flags);}

Dword index_unlock(Dword* object){Dword* resource=pointer(object[0]);using Unlock=Dword(__stdcall*)(Dword*);return reinterpret_cast<const Unlock*>(resource[0])[48/4](resource);}

void index_lost(Dword* object){if(object[0]){Dword* resource=pointer(object[0]);using Release=Dword(__stdcall*)(Dword*);reinterpret_cast<const Release*>(resource[0])[2](resource);object[0]=0;}}

void index_restore(Dword* object){if(!state::supports_hardware_vertex_programs())state::supports_hardware_fixed_function_processing();create_index(object);if(object[4]){Dword mapped;index_lock(object,0,object[2],&mapped,0);kernel::memory_copy(pointer(mapped),pointer(object[4]),object[2]);index_unlock(object);}}

void uses_construct(Dword* object,Dword type,Dword first,Dword second,Dword third){object[0]=type;object[3]=third;object[1]=first;object[2]=second;
    if(!object[4])object[4]=reinterpret_cast<Dword>(eagl::allocator(vertex_capacity*4,"VertexBufferType"));}

void uses_restore(Dword* object){if(!object[4])object[4]=reinterpret_cast<Dword>(eagl::allocator(vertex_capacity*4,"VertexBufferType"));}

Dword* vertex_fvf_construct(Dword* object,Dword value){object[0]=value;reinterpret_cast<std::uint8_t*>(object)[12]=1;object[1]=object[2]=0;object[4]=1;
    for(Dword* item=cache.vertex_head;item;item=pointer(item[5]))if(object[0]==item[0]){object[4]=item[4];++item[4];}
    object[6]=0;object[5]=reinterpret_cast<Dword>(cache.vertex_head);if(cache.vertex_head)cache.vertex_head[6]=reinterpret_cast<Dword>(object);cache.vertex_head=object;return object;}
// The program identifier at object[0] is retained on success.
Dword* pixel_construct(Dword* object,Dword data){object[1]=data;object[2]=1;if(data){using Create=Dword(__stdcall*)(Dword*,Dword,Dword*);reinterpret_cast<const Create*>(backend_device[0])[348/4](backend_device,data,object);
    for(Dword* item=cache.pixel_head;item;item=pointer(item[3]))if(object[0]==item[0]){object[2]=item[2];++item[2];}
    object[4]=0;object[3]=reinterpret_cast<Dword>(cache.pixel_head);if(cache.pixel_head)cache.pixel_head[4]=reinterpret_cast<Dword>(object);cache.pixel_head=object;}
    else{object[0]=object[4]=object[3]=0;}return object;}

void device_lost(){using Lost=void(__thiscall*)(Dword*);for(int i=0;i<vertex_capacity;++i){Dword* item=pointer(cache.vertices[i]);if(item)reinterpret_cast<const Lost*>(item[0])[1](item);}
    for(int i=0;i<index_capacity;++i)if(cache.indices[i])index_lost(pointer(cache.indices[i]));}

void device_restored(){using Restore=void(__thiscall*)(Dword*);for(int i=0;i<vertex_capacity;++i){Dword* item=pointer(cache.vertices[i]);if(item)reinterpret_cast<const Restore*>(item[0])[2](item);}
    for(int i=0;i<index_capacity;++i)if(cache.indices[i])index_restore(pointer(cache.indices[i]));}

void vertex_programs_lost(){using Delete=Dword(__stdcall*)(Dword*,Dword);for(Dword* item=cache.vertex_head;item;item=pointer(item[5]))if(!reinterpret_cast<std::uint8_t*>(item)[12]){
    --item[4];if(!item[4])reinterpret_cast<const Delete*>(backend_device[0])[312/4](backend_device,item[0]);item[0]=0;}}
}
