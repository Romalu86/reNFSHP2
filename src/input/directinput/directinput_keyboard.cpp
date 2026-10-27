#include "nfs/input/directinput/directinput.hpp"
#include "nfs/input/window/window_input.hpp"
#include <conio.h>
namespace nfs::input {
// The console reader is the actual initial callback.
static Dword console_key(){return _kbhit()?_getch():0;}
// only the low 16 bits of the queue value are returned.
static Dword queue_key(){return static_cast<std::uint16_t>(window_input::read_key_queue());}
KeyReader key_reader=console_key;

void keyboard_initialize(){bool ready=false;
    if(DirectInputCreateA(GetModuleHandleA(nullptr),1792,&keyboard_interface,nullptr)==0){IDirectInputDeviceA* initial;
        if(keyboard_interface->CreateDevice(keyboard_guid,&initial,nullptr)==0){
            HRESULT result=initial->QueryInterface(device2_guid,reinterpret_cast<void**>(&keyboard_device));initial->Release();
            if(result==0&&keyboard_device->SetDataFormat(&keyboard_format)==0&&keyboard_device->SetCooperativeLevel(window_input::handle(),6)==0)ready=true;}}
    if(!ready)keyboard_release();if(!keyboard_exit_registered){kernel::register_exit(keyboard_shutdown);keyboard_exit_registered=1;}
    keyboard_active=1;key_reader=queue_key;}

void keyboard_release(){if(keyboard_device){keyboard_device->Unacquire();keyboard_device->Release();keyboard_device=nullptr;}
    if(keyboard_interface){keyboard_interface->Release();keyboard_interface=nullptr;}}
// The exit-registration flag is not reset here.
void keyboard_shutdown(){key_reader=console_key;keyboard_release();kernel::unregister_exit(keyboard_shutdown);keyboard_active=0;}

void* keyboard_get(int state){if(state!=2){poll_state(keyboard_device,keyboard_state,256);return keyboard_state;}
    return describe_device(keyboard_device,&keyboard_about)==0?&keyboard_about:nullptr;}

Dword translate_key(int scan){if(scan==58){caps_lock=!caps_lock;return 0;}
    return translate_state(static_cast<const std::uint8_t*>(keyboard_get(6)),caps_lock,scan);}
}
