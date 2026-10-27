#include "nfs/graphics/resources/eagl_cleanup.hpp"
namespace nfs::graphics::resources {
static Dword* pointer(Dword p){return reinterpret_cast<Dword*>(p);}
static void release_com(Dword object){auto* p=pointer(object);using Release=Dword(__stdcall*)(Dword*);reinterpret_cast<const Release*>(p[0])[2](p);}
static void delete_object(Dword object){auto* p=pointer(object);reinterpret_cast<const gui::Destroy*>(p[0])[0](p,1);}

void index_destroy(Dword* object){if(object[0])release_com(object[0]);object[0]=0;object[4]=0;}

void uses_destroy(Dword* object){if(object[4]){eagl::deallocator(pointer(object[4]),vertex_capacity*4);object[4]=0;}}

void vertex_destroy(Dword* object){object[1]=object[2]=0;--object[4];if(object[4]==0&&reinterpret_cast<std::uint8_t*>(object)[12]==0){auto* d=backend_device;using Delete=Dword(__stdcall*)(Dword*,Dword);reinterpret_cast<const Delete*>(d[0])[312/4](d,object[0]);}
    if(object[6])pointer(object[6])[5]=object[5];if(object[5])pointer(object[5])[6]=object[6];if(cache.vertex_head==object)cache.vertex_head=pointer(object[5]);object[6]=object[5]=0;}

void pixel_destroy(Dword* object){object[1]=0;--object[2];if(object[2]==0&&object[0]){auto* d=backend_device;using Delete=Dword(__stdcall*)(Dword*,Dword);reinterpret_cast<const Delete*>(d[0])[360/4](d,object[0]);}
    if(object[4])pointer(object[4])[3]=object[3];if(object[3])pointer(object[3])[4]=object[4];if(cache.pixel_head==object)cache.pixel_head=pointer(object[3]);object[4]=object[3]=0;}

void destroy_cached_vertex(Dword* handle){Dword* object=pointer(*handle);if(object)for(int i=0;i<vertex_capacity;++i)if(cache.vertices[i]==reinterpret_cast<Dword>(object)){
    delete_object(reinterpret_cast<Dword>(object));*handle=0;cache.vertices[i]=0;--cache.vertex_count;return;}}

void release_vertex(Dword* handle,int stage){if(*handle){if(*handle==bound_streams[stage])bound_streams[stage]=0;
    for(int i=0;i<vertex_capacity;++i)if(cache.vertices[i]==*handle){Dword object=*handle;if(object)delete_object(object);*handle=0;cache.vertices[i]=0;--cache.vertex_count;return;}}}

void release_index(Dword* handle){if(*handle){if(*handle==bound_index)bound_index=0;for(int i=0;i<index_capacity;++i)if(cache.indices[i]==*handle){Dword* object=pointer(*handle);
    if(object){index_destroy(object);eagl::deallocator(object,20);}*handle=0;cache.indices[i]=0;--cache.index_count;return;}}}
// Capacity and global pointers are re-read after callbacks.
void shutdown(){if(cache.ready){for(int i=0;i<index_capacity;++i){Dword* object=pointer(cache.indices[i]);if(object){index_destroy(object);eagl::deallocator(object,20);--cache.index_count;}cache.indices[i]=0;}
    for(int i=0;i<vertex_capacity;++i){Dword object=cache.vertices[i];if(object){delete_object(object);--cache.vertex_count;}cache.vertices[i]=0;}
    for(int i=0;i<vertex_capacity;++i)uses_destroy(cache.uses+i*5);
    if(cache.indices)eagl::deallocator(cache.indices,index_capacity*4);if(cache.vertices)eagl::deallocator(cache.vertices,vertex_capacity*4);
    if(cache.uses)eagl::deallocator(cache.uses,20);if(cache.vertex_free_positions)eagl::deallocator(cache.vertex_free_positions,vertex_capacity*4);if(cache.index_free_positions)eagl::deallocator(cache.index_free_positions,index_capacity*4);cache.ready=0;}}

void shutdown_tail(){shutdown();}

void parameter_destroy(Dword* object){Dword* value=pointer(object[3]);if(object[2]==7){if(value&&object[3]!=4294967295u){pixel_destroy(value);eagl::deallocator(value,20);object[3]=0;}else object[3]=0;}
    else if(object[2]==6||object[2]==8){if(value){vertex_destroy(value);eagl::deallocator(value,28);}object[3]=0;}}

void parameter_list_destroy(Dword* object){for(int i=0;i<static_cast<int>(object[0]);++i){Dword* item=pointer(pointer(object[1])[i]);if(item){parameter_destroy(item);eagl::deallocator(item,16);}}
    if(object[1])eagl::deallocator(pointer(object[1]),object[0]*4);}

void parameter_block_destroy(Dword* object){for(int i=0;i<static_cast<int>(object[2]);++i){Dword* item=pointer(pointer(object[3])[i]);if(item){parameter_list_destroy(item);eagl::deallocator(item,8);}}
    if(object[3])gui::application_release(pointer(object[3]));for(int i=0;i<static_cast<int>(object[0]);++i){Dword* item=pointer(pointer(object[1])[i]);if(item){parameter_destroy(item);eagl::deallocator(item,16);}}
    if(object[1])eagl::deallocator(pointer(object[1]),object[0]*4);}

void dictionary_destroy(Dword* object){for(int i=0;i<static_cast<int>(object[2]);++i){Dword* item=pointer(pointer(object[4])[i]);if(item){parameter_block_destroy(item);eagl::deallocator(item,20);}}
    if(object[4]){eagl::deallocator(pointer(object[4]),object[2]*4);object[4]=0;}if(object[3]){eagl::deallocator(pointer(object[3]),object[2]*4);object[3]=0;}
    if(object[0]){eagl::deallocator(pointer(object[0]),object[1]*4);object[0]=0;}if(object[5]){eagl::deallocator(pointer(object[5]),object[7]*4);object[5]=0;}if(object[6]){eagl::deallocator(pointer(object[6]),object[7]*4);object[6]=0;}}

Dword technique_count(const Dword* object){return object[7];}

Dword technique_at(const Dword* object,int index){return pointer(object[6])[index];}
}
