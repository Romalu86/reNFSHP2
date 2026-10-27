#include "nfs/network/session/network_virtual.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::lifecycle::construction::network::entries {

void* allocate_payload(int bytes){void* result=kernel::memory_allocate(graphics::allocation_name,bytes,kernel::default_pool);if(gui::allocation_break_address&&result==gui::allocation_break_address)kernel::debug_break();return result;}

Dword __fastcall clone_payload(Dword* object,void*,Dword data,Dword){if(!object[1])return data;using Size=Dword(__thiscall*)(Dword*,Dword);using Copy=void(__thiscall*)(Dword*,Dword,Dword,Dword);
    Dword size=reinterpret_cast<const Size*>(object[0])[4](object,data);Dword result=reinterpret_cast<Dword>(allocate_payload(size));if(!result)return 0;
    Dword methods=object[0];size=reinterpret_cast<const Size*>(methods)[4](object,data);reinterpret_cast<const Copy*>(methods)[8](object,result,data,size);return result;}
}
