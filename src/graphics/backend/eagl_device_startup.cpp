#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/resources/files/file_cache.hpp"
extern "C" __declspec(dllimport) nfs::kernel::Dword* __stdcall Direct3DCreate8(UINT);
namespace nfs::graphics::backend {
Dword identity[16]{};  
static Dword reflection[4]={1065353216,3212836864u,1065353216,1065353216};
static Dword skinning[4]={1065353216,1073741824,1077936128,1082130432},zero_one_two_three[4]={0,1065353216,1073741824,1077936128};
static Dword shadow_colour[4]={1065353216,1008981770,1008981770,1008981770};
static Dword shadow_projection[16]={1065353216,0,0,0,0,1065353216,0,0,0,0,1065353216,0,0,0,0,1065353216};
static Dword environment[4]={1065353216,1048576000,1056964608,1061158912};
Dword fog_parameters[4]{};
extern Dword screen_info[4];

void construction_callbacks_register(eagl::CallbackPools* pools,const char* name,Dword construct,Dword destroy){eagl::pool_register(&pools->construct,name,construct);eagl::pool_register(&pools->destroy,name,destroy);}

void register_matrix_symbols(){using namespace unresolved;
    eagl::pool_register(&eagl::external_symbols,"EAGL::ViewPort::gpProjectionMatrix",reinterpret_cast<Dword>(projection_matrix));
    eagl::pool_register(&eagl::external_symbols,"EAGL::ViewPort::gpViewMatrix",reinterpret_cast<Dword>(view_matrix));
    eagl::pool_register(&eagl::external_symbols,"EAGL::ViewPort::gpModelMatrix",reinterpret_cast<Dword>(model_matrix));
    eagl::pool_register(&eagl::external_symbols,"EAGL::ViewPort::gpModelViewMatrix",reinterpret_cast<Dword>(model_view_matrix));
    eagl::pool_register(&eagl::external_symbols,"EAGL::ViewPort::gpViewProjectionMatrix",reinterpret_cast<Dword>(view_projection_matrix));
    eagl::pool_register(&eagl::external_symbols,"EAGL::ViewPort::gpModelViewProjectionMatrix",reinterpret_cast<Dword>(model_view_projection_matrix));}
 
std::uint8_t initialize_device(Device* device){using namespace unresolved;reinterpret_cast<std::uint8_t*>(device)[52]=1;lifecycle::construction::identity_matrix(identity);
    auto* v=files::file::build_info();int major=static_cast<int>(v->value_0),patch=static_cast<int>(v->value_4);
    if(major<430||major>501||(major==430&&patch<0)||(major==501&&patch<28)){
        eagl::report(0,"/// WARNING ///\n\t");eagl::report(0,major<421?
            "REAL version %i.%i patch %i to %i.%i patch %i is needed.  You are running on REAL version %i.%i build %i\n":
            "REAL version %i.%i patch %i to %i.%i patch %i is needed.  You are running on REAL version %i.%i patch %i\n",4,30,0,5,1,28,major/100,major%100,patch);
        eagl::report(0,"\tThe version of REAL you are using may be incompatible.\n\tProceed with Caution\n");}
    current_device=device;
    construction_callbacks_register(&eagl::constructors,"EAGL::TAR",reinterpret_cast<Dword>(tar::constructor),reinterpret_cast<Dword>(tar::destructor));
    construction_callbacks_register(&eagl::constructors,"RenderMethod",reinterpret_cast<Dword>(methods::initialize),reinterpret_cast<Dword>(resources::method_destroy));
    construction_callbacks_register(&eagl::constructors,"Model",reinterpret_cast<Dword>(methods::model_constructor),reinterpret_cast<Dword>(methods::model_destructor));
    eagl::callbacks_register(&eagl::runtime_allocators,"EAGL::GeoPrimState",reinterpret_cast<Dword>(geoprim::runtime_allocate),reinterpret_cast<Dword>(geoprim::runtime_release));
    eagl::callbacks_register(&eagl::runtime_allocators,"EAGL::TAR",reinterpret_cast<Dword>(tar::runtime_allocate),reinterpret_cast<Dword>(tar::runtime_release));register_matrix_symbols();
    eagl::pool_register(&eagl::external_symbols,"&EAGL::RenderMethodConstants::gReflectionConstants",reinterpret_cast<Dword>(reflection));
    eagl::pool_register(&eagl::external_symbols,"&EAGL::RenderMethodConstants::gSkinningConstants",reinterpret_cast<Dword>(skinning));
    eagl::pool_register(&eagl::external_symbols,"&EAGL::RenderMethodConstants::gZeroOneTwoThree",reinterpret_cast<Dword>(zero_one_two_three));
    eagl::pool_register(&eagl::external_symbols,"&EAGL::RenderMethodConstants::gShadowColour",reinterpret_cast<Dword>(shadow_colour));
    eagl::pool_register(&eagl::external_symbols,"&EAGL::RenderMethodConstants::gShadowProjectionMatrix",reinterpret_cast<Dword>(shadow_projection));
    eagl::pool_register(&eagl::external_symbols,"&EAGL::RenderMethodConstants::gEnvMapConstants",reinterpret_cast<Dword>(environment));
    eagl::pool_register(&eagl::external_symbols,"&EAGL::RenderMethodConstants::gScreenInfo",reinterpret_cast<Dword>(screen_info));
    eagl::pool_register(&eagl::external_symbols,"&EAGL::RenderMethodConstants::gFogParameters",reinterpret_cast<Dword>(fog_parameters));
    backend_interface=Direct3DCreate8(220);return backend_interface&&adapter_caps();}

void set_vertex_capacity(Device* object,Dword value){object->limits[6]=value;}

Dword vertex_capacity(Device* object){return object->limits[6];}

void set_index_capacity(Device* object,Dword value){object->limits[7]=value;}

Dword index_capacity(Device* object){return object->limits[7];}

Dword font_glyph_capacity(Device* object){return object->limits[32/4];}

std::uint8_t alternate_device(Device* object){return reinterpret_cast<std::uint8_t*>(object->limits)[40];}
}
