#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cmath>
#include <cstring>
namespace nfs::graphics::tar {
Dword mip_filter_override=~0u;
// Keep the native FLD/ST0 value transfer when the value is later stored to the FPU cache.
__declspec(noinline) static float number(Dword bits){float result;std::memcpy(&result,&bits,4);return result;}
// State and owner fields are read again after each backend call.
void bind(Dword* object){
    using namespace backend::unresolved;
    auto* shared=reinterpret_cast<Dword*>(object[9]);
    Dword stage=object[0],texture=shared[6];
    if(texture_values[stage]!=texture){
        texture_values[stage]=texture;
        backend::com(backend_device,244,object[0],texture);
    }
    shared=reinterpret_cast<Dword*>(object[9]);
    if(shared[8]!=~0u)backend::com(backend_device,272,shared[8]);
    for(int row=1;row<=3;++row){
        stage=object[0];Dword value=object[row+1];
        if(texture_filters[row][stage]!=value){
            texture_filters[row][stage]=value;
            backend::com(backend_device,252,object[0],Dword(row==3?25:row+12),object[row+1]);
        }
    }
    stage=object[0];
    if(texture_filters[6][stage]!=object[5]){
        texture_filters[6][stage]=object[5];
        backend::com(backend_device,252,object[0],Dword(21),object[5]);
    }
    Dword filter=state::texture_filter_override;
    if(filter!=~0u){
        stage=object[0];
        if(texture_filters[4][stage]!=filter){
            texture_filters[4][stage]=filter;
            backend::com(backend_device,252,object[0],Dword(16),filter);
            backend::com(backend_device,252,object[0],Dword(17),state::texture_filter_override);
        }
    }else{
        stage=object[0];filter=object[6];
        if(texture_filters[4][stage]!=filter){
            texture_filters[4][stage]=filter;
            backend::com(backend_device,252,object[0],Dword(16),object[6]);
            backend::com(backend_device,252,object[0],Dword(17),object[6]);
        }
    }
    Dword mip=mip_filter_override;
    if(mip!=~0u){
        stage=object[0];
        if(texture_filters[5][stage]!=mip){
            texture_filters[5][stage]=mip;
            backend::com(backend_device,252,object[0],Dword(18),mip);
        }
    }else{
        stage=object[0];
        mip=object[8];
        if(texture_filters[5][stage]!=mip){
            texture_filters[5][stage]=mip;
            backend::com(backend_device,252,object[0],Dword(18),object[8]);
        }
    }
    float override_value=number(state::texture_lod_bias_bits_override);
    stage=object[0];
    float cached=number(texture_objects[stage]);
    if(override_value!=0){
        if(cached!=number(state::texture_lod_bias_bits_override)){
            const float stored=number(state::texture_lod_bias_bits_override);
            const Dword argument=state::texture_lod_bias_bits_override;

            *reinterpret_cast<volatile float*>(texture_objects+stage)=stored;
            backend::com(backend_device,252,object[0],Dword(19),argument);
        }
    }else if(cached!=number(object[7])){
        texture_objects[stage]=object[7];
        backend::com(backend_device,252,object[0],Dword(19),object[7]);
    }
}
}
