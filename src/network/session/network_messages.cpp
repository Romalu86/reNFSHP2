#include "nfs/network/session/network_virtual.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::lifecycle::construction::network::entries {
static Dword* pointer(Dword value){return reinterpret_cast<Dword*>(value);}
static std::uint16_t short_at(Dword value,int offset=0){return *reinterpret_cast<const std::uint16_t*>(value+offset);}

Dword __fastcall forward_message(Dword*,void*,Dword data,Dword,Dword,Dword target){nfs::network::forward(reinterpret_cast<Dword*>(lifecycle::manager->slots[27]),target,data,0);return 1;}

Dword __fastcall server_second_dispatch_adjust(Dword* object,void*,Dword data,Dword first,Dword second,Dword packet){(void)object;(void)first;(void)second;
    if(short_at(packet)==1&&pointer(data)[3]&&(!pointer(packet)[4]||short_at(pointer(packet)[4])!=short_at(data))&&(!(pointer(packet)[3]&32)||!pointer(data)[1]))nfs::network::send_packet(data,pointer(packet)[1],pointer(packet)[2],pointer(packet)[3]);return 1;}

Dword __fastcall server_compare_adjust(Dword*,void*,Dword first,Dword second){return nfs::network::transport_matches_route(reinterpret_cast<void*>(first),reinterpret_cast<void*>(second));}
}
