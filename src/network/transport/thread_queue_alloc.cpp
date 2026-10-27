#include "nfs/network/transport/thread_queue.hpp"
namespace nfs::lifecycle::construction::network::queue {
static Dword* pointer(Dword value){return reinterpret_cast<Dword*>(value);}
static void enter(Dword* object){if(object[15])EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(object+9));}
static void leave(Dword* object){if(object[15])LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(object+9));}

Dword append(Dword* object,Dword data,Dword context){auto* node=static_cast<Dword*>(entries::allocate_payload(20));if(!node)return 0;for(int i=0;i<5;++i)node[i]=0;
    using Clone=Dword(__thiscall*)(Dword*,Dword,Dword);node[4]=reinterpret_cast<const Clone*>(object[0])[6](object,data,context);enter(object);
    if(object[2]&&static_cast<int>(object[2])<=static_cast<int>(object[7])){Dword* old=pointer(object[5]);if(old){unlink(object,old);using Destroy=void(__thiscall*)(Dword*,Dword);reinterpret_cast<const Destroy*>(object[0])[7](object,old[4]);gui::application_release(old);}}
    node[0]=object[6];++object[7];if(object[6])pointer(object[6])[1]=reinterpret_cast<Dword>(node);object[6]=reinterpret_cast<Dword>(node);if(!object[5])object[5]=reinterpret_cast<Dword>(node);
    object[16]=1;node[1]=0;leave(object);return 1;}
}
