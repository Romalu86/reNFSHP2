#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::techniques {

void select(Dword* object,Dword group){int index=0;while(group<object[7]){for(;index<static_cast<int>(object[2]);++index)if(reinterpret_cast<std::uint8_t*>(object[3])[index]){reinterpret_cast<Dword*>(object[6])[group]=index;reinterpret_cast<Dword*>(object[5])[group]=reinterpret_cast<Dword*>(object[4])[index];break;}for(;index<static_cast<int>(object[2]);++index)if(reinterpret_cast<Dword*>(reinterpret_cast<Dword*>(object[4])[index])[4]==group+1)break;++group;}}
}
