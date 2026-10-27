#include "nfs/graphics/resources/eagl_objects.hpp"
namespace nfs::graphics {
HWND render_window=nullptr;  

Dword* context_set_owner(Dword* object,Dword* owner){object[1]=reinterpret_cast<Dword>(owner);return object;}
// Receives the state at complete render_context+8.
Dword* context_state_construct(Dword* object,Dword* owner){object[0]=reinterpret_cast<Dword>(owner);object[24]=15;object[31]=0;object[42]=256;
    for(int i=48;i<=53;++i)object[i]=0;return object;}
// IAT  imports Sleep.
void context_default_error(){Sleep(1);}
 
void context_default_callback(){}
 
void context_default_close(){SendMessageA(render_window,16,0,0);}

Dword* construct_context(Dword* object,Device* owner_device){context_set_owner(object,object);context_state_construct(object+2,object);object[224/4]=reinterpret_cast<Dword>(owner_device);
    for(int i=32/4;i<=52/4;++i)object[i]=0;object[56/4]=1;object[24/4]=object[28/4]=0;
    auto* bytes=reinterpret_cast<std::uint8_t*>(object);bytes[60]=1;object[64/4]=3;bytes[68]=0;
    for(int i=72/4;i<=84/4;++i)object[i]=1;object[88/4]=8;object[92/4]=0;object[96/4]=object[100/4]=4294967295u;
    bytes[108]=0;bytes[109]=1;bytes[110]=0;object[112/4]=object[116/4]=object[120/4]=0;
    object[124/4]=object[128/4]=1065353216;object[132/4]=object[152/4]=0;object[156/4]=4294967295u;object[160/4]=0;object[164/4]=1;
    object[3]=object[4]=object[5]=0;bytes[168]=1;object[172/4]=0;
    auto* name=static_cast<char*>(eagl::allocator(object[176/4],"Window Name"));object[180/4]=reinterpret_cast<Dword>(name);
    const char initial[]="EAGL PC";for(int i=0;i<8;++i)name[i]=initial[i];object[136/4]=1;bytes[184]=0;object[140/4]=0;
    object[188/4]=reinterpret_cast<Dword>(context_default_error);object[192/4]=reinterpret_cast<Dword>(context_default_callback);object[196/4]=reinterpret_cast<Dword>(context_default_close);object[148/4]=0;return object;}
}
