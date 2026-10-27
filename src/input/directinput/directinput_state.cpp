#include "nfs/input/directinput/directinput.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
namespace nfs::input {
DeviceSlot devices[32]{};int device_count=0;IDirectInputA* devices_interface=nullptr;
IDirectInputA* keyboard_interface=nullptr;IDirectInputDevice2A* keyboard_device=nullptr;
IDirectInputA* mouse_interface=nullptr;IDirectInputDevice2A* mouse_device=nullptr;
std::uint8_t keyboard_state[256]{};DeviceAbout keyboard_about{},mouse_about{};
DIMOUSESTATE mouse_state{};int mouse_minimum[3]{},mouse_maximum[3]{},mouse_wheel=0;
int mouse_access=4,mouse_axis_mode=2,mouse_clip=1,mouse_registered=0,mouse_exit_registered=0;
int keyboard_active=0,keyboard_exit_registered=0;bool caps_lock=false;
// 32 DIEFFECT records use a 148-byte stride, with COM and owner fields at offsets 140 and 144.
// Initialization state and keyboard backup follow the records.
alignas(4) static std::uint8_t effect_storage[5000]{};
std::uint8_t* effect_record_data(){return effect_storage;}
EffectSlot* effect_slots(){return reinterpret_cast<EffectSlot*>(effect_storage+140);}
Dword& effects_initialized(){return *reinterpret_cast<Dword*>(effect_storage+4736);}
std::uint8_t* state_backup(){return effect_storage+4744;}
Dword& effect_stop_state(int index){return *reinterpret_cast<Dword*>(effect_storage+132+index*148);}
// Poll/Acquire and GetDeviceState/Acquire are separate retries.
HRESULT poll_state(IDirectInputDevice2A* device,void* state,Dword size){
    HRESULT result=device->Poll();
    if((result==HRESULT(2147942412u)||result==HRESULT(2147942430u))&&device->Acquire()==0)device->Poll();
    kernel::memory_copy(state_backup(),state,size);
    result=device->GetDeviceState(size,state);
    if(result==HRESULT(2147942412u)||result==HRESULT(2147942430u)){
        result=device->Acquire();if(result!=0){kernel::memory_copy(state,state_backup(),size);return result;}
        result=device->GetDeviceState(size,state);
    }
    if(result!=0)kernel::memory_copy(state,state_backup(),size);return result;
}
// The masks describe offsets accepted by GetObjectInfo.
HRESULT describe_device(IDirectInputDevice2A* device,DeviceAbout* about){
    kernel::memory_fill(about,0,sizeof(*about));DIDEVICEINSTANCEA instance;instance.dwSize=580;
    HRESULT result=device->GetDeviceInfo(&instance);if(result!=0)return result;
    about->type=instance.dwDevType;std::strncpy(about->instance,instance.tszInstanceName,259);
    std::strncpy(about->product,instance.tszProductName,259);
    DIDEVCAPS capabilities;capabilities.dwSize=44;result=device->GetCapabilities(&capabilities);if(result!=0)return result;
    about->flags=(capabilities.dwFlags&1)|((capabilities.dwFlags>>7)&2);DIDEVICEOBJECTINSTANCEA object;object.dwSize=316;
    Dword found=0;for(Dword offset=0;found<capabilities.dwButtons&&offset<32;++offset){
        Dword type=about->type&255;
        if((type==2&&device->GetObjectInfo(&object,offset+12,1)==0)||(type==4&&device->GetObjectInfo(&object,offset+48,1)==0)){
            about->buttons|=Dword(1)<<offset;++found;}}
    const Dword axes[]={0,4,8,12,16,20,24,28};found=0;
    for(Dword axis=0;found<capabilities.dwAxes&&axis<8;++axis)if(device->GetObjectInfo(&object,axes[axis],1)==0){++found;about->axes|=Dword(1)<<axis;}
    found=0;for(Dword pov=0;found<capabilities.dwPOVs&&pov<4;++pov)if(device->GetObjectInfo(&object,32+pov*4,1)==0){++found;about->povs|=Dword(1)<<pov;}
    return result;
}

void effects_initialize(){if(!effects_initialized()){for(int i=0;i<32;++i)effect_slots()[i].device_index=-1;effects_initialized()=1;}}

bool send_feedback_command(IDirectInputDevice2A* device,Dword command){HRESULT result=device->SendForceFeedbackCommand(command);
    if(result==HRESULT(2147942412u)||result==HRESULT(2147942430u)||result==HRESULT(2147746309u)){
        result=device->Acquire();if(result==0)result=device->SendForceFeedbackCommand(command);}return result==0;}
// consumes the AL result of the final call.
bool reset_effects(int index){for(int i=0;i<32;++i)if(effect_slots()[i].device_index==index)effect_slots()[i].device_index=-1;return send_feedback_command(devices[index].device,1);}

void disable_feedback(int index){if(devices_interface&&index>=0&&index<device_count&&devices[index].device&&(devices[index].flags&1)){
    reset_effects(index);devices[index].flags&=~Dword(1);}}
// Inactive slots are deliberately not released or cleared.
void effects_shutdown(){if(effects_initialized()){
    for(int i=0;i<32;++i)if(effect_slots()[i].device_index!=-1)effect_slots()[i].effect->Release();
    for(int i=0;i<32;++i)if(i<device_count&&devices[i].device)disable_feedback(i);}}
}
