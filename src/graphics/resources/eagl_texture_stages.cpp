#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::graphics::backend {
// No synthetic clamp on the driver's stage count.
void apply_texture_stages(const TextureStageState* input){kernel::memory_copy(&texture_stage_snapshot,input,548);int limit=static_cast<int>(adapter_caps()[696/4]-1u);int stage=0;
    static const Dword names[17]={1,26,2,3,4,27,5,6,28,7,8,9,10,11,22,23,24};
    for(;stage<=limit;++stage)for(int row=0;row<17;++row){Dword value=input->values[row][stage];texture_stage_state.values[row][stage]=value;com(backend_device,252,stage,names[row],input->values[row][stage]);}
    for(;stage<8;++stage){texture_stage_state.values[0][stage]=1;com(backend_device,252,stage,Dword(1),Dword(1));texture_stage_state.values[4][stage]=1;com(backend_device,252,stage,Dword(4),Dword(1));Dword value=input->values[13][stage];texture_stage_state.values[13][stage]=value;com(backend_device,252,stage,Dword(11),value);}}

void initialize_texture_stages(){for(int stage=0;stage<8;++stage){Dword values[17]={Dword(stage==0?4:1),1,2,1,Dword(stage==0?2:1),1,2,1,1,0,0,0,0,Dword(stage),0,0,0};
    for(int row=0;row<17;++row)texture_stage_state.values[row][stage]=values[row];}texture_stage_state.tail=0;apply_texture_stages(&texture_stage_state);}
}
