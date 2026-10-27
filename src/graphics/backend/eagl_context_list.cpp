#include "nfs/graphics/resources/eagl_objects.hpp"
namespace nfs::graphics {

Dword* create_context(Device* owner_device){auto* object=static_cast<Dword*>(eagl::allocator(228,"RenderContext new"));if(object)construct_context(object,owner_device);
    auto* fields=reinterpret_cast<Dword*>(owner_device);*reinterpret_cast<volatile Dword*>(reinterpret_cast<std::uintptr_t>(object)+216)=fields[11];fields[11]=reinterpret_cast<Dword>(object);current_context=object;return object;}

void set_context_mode(Dword* render_context,Dword value){reinterpret_cast<std::uint8_t*>(render_context)[60]=static_cast<std::uint8_t>(value);}
}
