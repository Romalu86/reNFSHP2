#include "nfs/graphics/resources/eagl_cleanup.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
namespace nfs::movies { void release_movie_device_textures(); }
namespace nfs::graphics {
Dword* backend_interface=nullptr;Dword* backend_device=nullptr;  
std::uint8_t window_created=0;HINSTANCE& window_instance=backend::window_definition.hInstance;const char*& window_class=backend::window_definition.lpszClassName; // These class fields share overlapping storage.
static void release_com(Dword* p){using Release=Dword(__stdcall*)(Dword*);reinterpret_cast<const Release*>(p[0])[2](p);}

void context_base_destroy(Dword*){}
// Viewport list head is re-read after each deletion.
void context_contents_shutdown(Dword* object){for(Dword* method=resources::render_methods;method;method=reinterpret_cast<Dword*>(method[4]))resources::method_destroy(method);
    while(object[212/4])destroy_viewport(object,reinterpret_cast<Dword*>(object[212/4]));
    if(object[204/4])release_com(reinterpret_cast<Dword*>(object[204/4]));if(object[200/4])release_com(reinterpret_cast<Dword*>(object[200/4]));
    movies::release_movie_device_textures();resources::shutdown();resources::shutdown_tail();if(backend_device)release_com(backend_device);backend_device=nullptr;
    if(window_created){if(render_window){DestroyWindow(render_window);render_window=nullptr;}UnregisterClassA(window_class,window_instance);window_created=0;}
    eagl::deallocator(reinterpret_cast<void*>(object[180/4]),object[176/4]);object[180/4]=0;context_base_destroy(object);}
}
namespace nfs::graphics::targets {
Dword* first_links=nullptr;Dword* second_links=nullptr;  
static Dword* pointer(Dword value){return reinterpret_cast<Dword*>(value);}

void viewport_destroy(Dword* object,Dword* viewport){Dword* removed=pointer(object[10]);if(viewport==removed)object[10]=removed[364/4];else{Dword* previous=removed;
    while(previous&&previous[364/4]!=reinterpret_cast<Dword>(viewport))previous=pointer(previous[364/4]);removed=pointer(previous[364/4]);previous[364/4]=removed[364/4];}
    viewports::destroy(removed);eagl::deallocator(removed,372);}
static void detach(Dword*& head,Dword* owner){for(Dword* item=head;item;item=pointer(item[3]))if(item[0]==reinterpret_cast<Dword>(owner)){
    if(item[2])pointer(item[2])[3]=item[3];if(item[3])pointer(item[3])[2]=item[2];if(head==item)head=nullptr;
    item[0]=item[1]=item[2]=item[3]=0;eagl::deallocator(item,16);break;}}
// Removing a head clears the global pointer.
void detach_links(Dword* owner){detach(first_links,owner);detach(second_links,owner);}

void surface_destroy(Dword** owner){Dword* object=*owner;while(object[10])viewport_destroy(object,pointer(object[10]));
    if(object[13]){Dword* resource=pointer(object[13]);using Release=Dword(__stdcall*)(Dword*);reinterpret_cast<const Release*>(resource[0])[2](resource);(*owner)[13]=0;}}

void contents_destroy(Dword* object){while(object[10])viewport_destroy(object,pointer(object[10]));detach_links(object+1);surface_destroy(reinterpret_cast<Dword**>(object));}

void destroy(Device* owner_device,Dword* target){Dword* owner=owner_device->limits;Dword* removed=pointer(owner[12]);if(target==removed)owner[12]=removed[12];else{Dword* previous=removed;
    while(previous&&previous[12]!=reinterpret_cast<Dword>(target))previous=pointer(previous[12]);removed=pointer(previous[12]);previous[12]=removed[12];}
    contents_destroy(removed);eagl::deallocator(removed,212);}
}
