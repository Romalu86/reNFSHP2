#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
#include <cstring>
namespace nfs::graphics::geoprim {
Dword current_primitive=0;std::uint8_t depth_override=1;  

std::uint8_t apply(Dword* object){using namespace backend::unresolved;current_primitive=object[0];
    if(!object[9])for(int i=0;i<8;++i){if(texture_values[i])backend::com(backend_device,244,i,Dword(0));texture_values[i]=0;}
    if(backend::depth_write_state!=object[16]){if(object[16]==4294967295u){std::uint8_t value;state::read_depth_write(backend::selected_context(backend::selected_device()),&value);backend::set_depth_write(backend::selected_context(backend::selected_device()),value);}
        else{depth_override=static_cast<std::uint8_t>(object[16]&1);backend::depth_write_state=depth_override;backend::com(backend_device,200,Dword(14),object[16]);}}
    bool equal=true;for(int i=0;i<17;++i)if(material_state[i]!=object[i]){equal=false;break;}if(equal)return 1;
    kernel::memory_copy(material_state,object,68);auto render=[&](Dword name,Dword value){backend::com(backend_device,200,name,value);};Dword* cached=render_states;
    float segments;std::memcpy(&segments,object+15,4);
    auto retained=static_cast<long double>(static_cast<int>(render_dirty));
    auto requested=static_cast<long double>(segments);
    if(std::isnan(segments)||retained<requested||requested<retained){
        auto value=static_cast<long long>(requested);
        render_dirty=Dword(value);
        render(164,object[15]);render(163,0);}
    if(cached[13]!=object[11]){cached[13]=object[11];render(8,object[11]);}if(cached[14]!=object[12]){cached[14]=object[12];render(171,object[12]);}
    if(cached[2]!=object[2]||cached[3]!=object[3]){cached[2]=object[2];cached[3]=static_cast<const volatile Dword*>(object)[3];if(!object[2])render(22,1);else if(!object[3])render(22,2);else if(object[3]==1)render(22,3);}
    if(cached[1]!=object[1]){cached[1]=object[1];if(object[1]==0){render(9,1);render(29,0);}else if(object[1]==1){render(9,2);render(29,0);}else if(object[1]==2){render(9,2);render(29,1);}}
    if(cached[4]!=object[4]){cached[4]=object[4];render(23,object[4]);}
    if(cached[7]!=object[5]||cached[5]!=object[13]||cached[6]!=object[14]){cached[7]=object[5];cached[5]=static_cast<const volatile Dword*>(object)[13];cached[6]=static_cast<const volatile Dword*>(object)[14];render(19,object[13]);render(20,object[14]);}
    if(cached[8]!=object[6]){cached[8]=object[6];render(15,object[6]);}if(cached[9]!=object[7]){cached[9]=object[7];render(24,object[7]);}if(cached[10]!=object[8]){cached[10]=object[8];render(25,object[8]);}
    if(cached[12]!=object[10]){cached[12]=object[10];if(!object[10])render(27,0);else if(object[10]==1)render(27,1);}return 1;}
}
