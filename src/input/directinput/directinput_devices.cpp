#include "nfs/input/directinput/directinput.hpp"
#include "nfs/input/window/window_input.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::input {
// stdcall callback with two stack arguments. Failed slots are reused without releasing a
// partially created device.
BOOL CALLBACK enumerate_device(const DIDEVICEINSTANCEA* instance,void* context){int count=*static_cast<int*>(context);DeviceSlot& slot=devices[count];
    slot.device=nullptr;slot.flags&=~Dword(1);IDirectInputDeviceA* initial;
    if(devices_interface->CreateDevice(instance->guidInstance,&initial,nullptr)==0){
        HRESULT result=initial->QueryInterface(device2_guid,reinterpret_cast<void**>(&slot.device));initial->Release();
        if(result==0){slot.type=instance->dwDevType;Dword type=slot.type&255;const DIDATAFORMAT* format=&joystick_format;DWORD access=6;
            if(type==2)format=&mouse_format;else if(type==3)format=&keyboard_format;else if(type==4)access=5;
            if(slot.device->SetDataFormat(format)==0&&slot.device->SetCooperativeLevel(window_input::handle(),access)==0)++count;}}
    *static_cast<int*>(context)=count;return count<32;}
// The EnumDevices count is committed only on success.
Dword devices_initialize(){if(devices_interface)return 1;kernel::memory_fill(devices,0,8576);
    if(DirectInputCreateA(GetModuleHandleA(nullptr),1792,&devices_interface,nullptr)==0){int count=0;
        if(devices_interface->EnumDevices(0,enumerate_device,&count,0)==0){device_count=count;effects_initialize();kernel::register_exit(devices_shutdown);return 1;}}
    devices_shutdown();kernel::error("INPUT_init - Error initializing\n");return 0;}

void devices_shutdown(){effects_shutdown();for(int i=0;i<device_count;++i)if(devices[i].device){devices[i].device->Unacquire();devices[i].device->Release();devices[i].device=nullptr;}
    if(devices_interface){devices_interface->Release();devices_interface=nullptr;}}
}
