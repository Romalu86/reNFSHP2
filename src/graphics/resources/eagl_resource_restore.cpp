#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstring>
namespace nfs::graphics::backend {

void pixel_programs_lost(){invalidate_pixel_shader_cache();for(Dword* item=resources::cache.pixel_head;item;item=reinterpret_cast<Dword*>(item[3])){--item[2];if(!item[2]&&item[0])com(backend_device,360,item[0]);}}

void pixel_programs_restored(){invalidate_pixel_shader_cache();for(Dword* item=resources::cache.pixel_head;item;item=reinterpret_cast<Dword*>(item[3])){if(item[1])com(backend_device,348,item[1],item);++item[2];}}

void targets_lost(){for(Dword* link=targets::first_links;link;link=reinterpret_cast<Dword*>(link[3])){Dword* target=reinterpret_cast<Dword*>(link[0]);if(target[12]){com(reinterpret_cast<Dword*>(target[12]),8);target[12]=0;}}}

void targets_restored(){Dword* link=targets::first_links;while(link){Dword* next=reinterpret_cast<Dword*>(link[3]);Dword previous=link[2];if(link[1]){
    Dword* texture=reinterpret_cast<Dword*>(reinterpret_cast<Dword*>(link[1])[36/4]);Dword* resource=reinterpret_cast<Dword*>(texture[6]);if(!resource){
        if(targets::first_links==link){targets::first_links=next;if(next)next[2]=0;}else{if(previous)reinterpret_cast<Dword*>(previous)[3]=reinterpret_cast<Dword>(next);if(link[3])reinterpret_cast<Dword*>(link[3])[2]=previous;}
        link[0]=link[1]=link[2]=link[3]=0;eagl::deallocator(link,16);
    }else com(resource,60,Dword(0),reinterpret_cast<Dword*>(link[0])+12);}link=next;}}
 
void depth_targets_lost(){}
// Authorized graphics compatibility: preserve the next

// after the native path clears and releases its current binding.
void depth_targets_restored() {
    Dword *binding = targets::second_links;
    while (binding) {
        if (binding[1]) {
            Dword *texture = reinterpret_cast<Dword *>(
                reinterpret_cast<Dword *>(binding[1])[9]);
            const Dword resource = texture[7];
            if (!resource) {
                Dword *next = reinterpret_cast<Dword *>(binding[3]);
                // This is a borrowed surface pointer, not another COM reference.
                // buffers_lost has already released the owning TAR surface.
                reinterpret_cast<Dword *>(binding[0])[13] = 0;
                if (binding[2])
                    reinterpret_cast<Dword *>(binding[2])[3] = binding[3];
                if (binding[3])
                    reinterpret_cast<Dword *>(binding[3])[2] = binding[2];
                if (targets::second_links == binding)
                    targets::second_links = next;
                binding[0] = binding[1] = binding[2] = binding[3] = 0;
                eagl::deallocator(binding, 16);
                binding = next;
                continue;
            }
            reinterpret_cast<Dword *>(binding[0])[13] = resource;
        }
        binding = reinterpret_cast<Dword *>(binding[3]);
    }
}
}
