#include "nfs/graphics/resources/eagl_cleanup.hpp"
#include <initializer_list>
namespace nfs::graphics {

void device_state_destroy(Dword*){}

void device_base_destroy(Dword*){}

void construction_callbacks_remove(eagl::CallbackPools* pools,const char* name){eagl::pool_remove(&pools->construct,name);eagl::pool_remove(&pools->destroy,name);}

void device_shutdown(Device* object){for(const char* name:{"&EAGL::RenderMethodConstants::gReflectionConstants","&EAGL::RenderMethodConstants::gSkinningConstants",
    "&EAGL::RenderMethodConstants::gZeroOneTwoThree","&EAGL::RenderMethodConstants::gShadowColour","&EAGL::RenderMethodConstants::gShadowProjectionMatrix",
    "&EAGL::RenderMethodConstants::gEnvMapConstants","&EAGL::RenderMethodConstants::gScreenInfo","&EAGL::RenderMethodConstants::gFogParameters"})eagl::pool_remove(&eagl::external_symbols,name);
    for(const char* name:{"EAGL::TAR","RenderMethod","Model","DataBuffer"})construction_callbacks_remove(&eagl::constructors,name);
    eagl::callbacks_remove(&eagl::runtime_allocators,"EAGL::TAR");eagl::callbacks_remove(&eagl::runtime_allocators,"EAGL::GeoPrimState");
    eagl::pool_clear(&eagl::external_symbols);eagl::callbacks_clear(&eagl::runtime_allocators);eagl::callbacks_clear(&eagl::constructors);
    auto* fields=reinterpret_cast<Dword*>(object);while(fields[11])destroy_context(object,reinterpret_cast<Dword*>(fields[11]));
    while(fields[12])targets::destroy(object,reinterpret_cast<Dword*>(fields[12]));
    using Release=Dword(__stdcall*)(Dword*);reinterpret_cast<const Release*>(backend_interface[0])[2](backend_interface);backend_interface=nullptr;
    device_state_destroy(fields+1);device_base_destroy(fields);}
}
