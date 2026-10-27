#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::graphics::techniques {

std::uint8_t pass_validate(Dword* pass){for(int i=0;i<static_cast<int>(*pass);++i)if(!parameter_validate(reinterpret_cast<Dword*>(reinterpret_cast<Dword*>(pass[1])[i])))return 0;pass_apply(pass);Dword passes;int result=backend::com(backend_device,256,&passes);return result==0&&passes==1;}

std::uint8_t technique_validate(Dword* technique){for(Dword i=0;i<device_caps[(8692204-8692056)/4];++i)backend::com(backend_device,244,i,Dword(0));if(!caps_ready){kernel::memory_copy(device_caps,backend::adapter_caps()+548/4,53*4);caps_ready=1;}for(int i=0;i<static_cast<int>(*technique);++i)if(!parameter_validate(reinterpret_cast<Dword*>(reinterpret_cast<Dword*>(technique[1])[i])))return 0;for(Dword i=0;i<device_caps[(8692204-8692056)/4];++i)backend::com(backend_device,244,i,Dword(0));for(int i=0;i<static_cast<int>(technique[2]);++i)if(!pass_validate(reinterpret_cast<Dword*>(reinterpret_cast<Dword*>(technique[3])[i])))return 0;return 1;}
}
