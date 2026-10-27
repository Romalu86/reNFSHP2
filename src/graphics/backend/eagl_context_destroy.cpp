#include "nfs/graphics/resources/eagl_cleanup.hpp"
namespace nfs::graphics {

void destroy_context(Device* owner_device,Dword* render_context){auto* fields=reinterpret_cast<Dword*>(owner_device);auto* removed=reinterpret_cast<Dword*>(fields[11]);
    if(render_context==removed)fields[11]=removed[216/4];else{Dword* previous=removed;while(previous&&previous[216/4]!=reinterpret_cast<Dword>(render_context))previous=reinterpret_cast<Dword*>(previous[216/4]);
        removed=reinterpret_cast<Dword*>(previous[216/4]);previous[216/4]=removed[216/4];}
    context_contents_shutdown(removed);eagl::deallocator(removed,228);if(current_context==render_context)current_context=reinterpret_cast<Dword*>(fields[11]);}
}
