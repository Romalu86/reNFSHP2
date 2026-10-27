#include "nfs/input/directinput/directinput.hpp"
#include "nfs/input/window/window_input.hpp"
namespace nfs::input {
// six-argument stdcall message callback, RET 18h.
static LONG add_mouse_delta(LONG value, LONG delta) {
    return std::int32_t(Dword(value) + Dword(delta));
}
static Dword __stdcall wheel_message(Dword enabled, HWND window, Dword, WPARAM wparam, LPARAM,
                                    Dword) {
    if (enabled && window_input::handle() == window)
        mouse_wheel = add_mouse_delta(mouse_wheel, std::int16_t(wparam >> 16));
    return 0;
}
// Failure still performs the common state/exit initialization.
void mouse_initialize(){if(!mouse_registered){mouse_registered=1;window_input::register_message(522,wheel_message);}
    if(!mouse_interface){bool ready=false;if(DirectInputCreateA(GetModuleHandleA(nullptr),1792,&mouse_interface,nullptr)==0){IDirectInputDeviceA* initial;
        if(mouse_interface->CreateDevice(mouse_guid,&initial,nullptr)==0){HRESULT result=initial->QueryInterface(device2_guid,reinterpret_cast<void**>(&mouse_device));initial->Release();
            if(result==0&&mouse_device->SetDataFormat(&mouse_format)==0){DIPROPDWORD property{{20,16,0,0},1};
                ready=mouse_device->SetProperty(DIPROP_AXISMODE,&property.diph)==0;}}}if(!ready)mouse_release();}
    mouse_state={};mouse_set(8,2);mouse_set(9,1);if(mouse_interface)mouse_set(7,4);
    if(!mouse_exit_registered){kernel::register_exit(mouse_shutdown);mouse_exit_registered=1;}
    POINT cursor;GetCursorPos(&cursor);if(ScreenToClient(window_input::handle(),&cursor)){mouse_set(3,cursor.x);mouse_set(4,cursor.y);}}

void mouse_release(){if(mouse_device){mouse_device->Unacquire();mouse_device->Release();mouse_device=nullptr;}
    if(mouse_interface){mouse_interface->Release();mouse_interface=nullptr;}}
// Does not clear the exit flag or unregister the window callback.
void mouse_shutdown(){mouse_release();mouse_registered=0;}

bool mouse_set(int state,int value){switch(state){
    case 3:mouse_state.lX=value;if(mouse_access==4)window_input::position_cursor(value,mouse_state.lY);return true;
    case 4:mouse_state.lY=value;if(mouse_access==4)window_input::position_cursor(mouse_state.lX,value);return true;
    case 5:mouse_state.lZ=value;return true;
    case 7:{bool success=true;if(mouse_access!=value){if(!mouse_interface){kernel::error("MOUSE_setstate(INPUT_STATE_ACCESSMODE) - Cannot change the access mode as Direct Input was not initialized correctly\n");mouse_access=value;return true;}
        if(value==4&&mouse_access!=4){mouse_device->Unacquire();window_input::position_cursor(mouse_state.lX,mouse_state.lY);mouse_access=value;return true;}
        DWORD flags=(value!=5?2u:1u)|4u;success=mouse_device->SetCooperativeLevel(window_input::handle(),flags)==0;
        if(!success){HRESULT result=mouse_device->Unacquire();if(result==0||result==1){success=mouse_device->SetCooperativeLevel(window_input::handle(),flags)==0;mouse_device->Acquire();}}}
        mouse_access=value;return success;}
    case 8:mouse_axis_mode=value;return true;case 9:mouse_clip=value;return true;
    default:kernel::error("[MOUSE] - Invalid setstate\n");return false;}}
static LONG clip_axis(LONG value,int axis){LONG lower=value<=mouse_minimum[axis]?mouse_minimum[axis]:value;
    if(mouse_maximum[axis]<lower)return mouse_maximum[axis];if(value<=mouse_minimum[axis])return mouse_minimum[axis];return value;}
// Numeric states 7..9 are returned in the pointer-sized result.
void* mouse_get(Dword state){switch(state){
    case 2:if(!mouse_interface){kernel::error("MOUSE_getstate(INPUT_STATE_ABOUT) - Error initializing Direct Input\n");return nullptr;}
        return describe_device(mouse_device,&mouse_about)==0?&mouse_about:nullptr;
    case 6:if(mouse_access==4){Dword buttons;window_input::cursor_state(&buttons,&mouse_state.lX,&mouse_state.lY);
        mouse_state.lZ=add_mouse_delta(mouse_state.lZ,mouse_wheel);if(mouse_clip)mouse_state.lZ=clip_axis(mouse_state.lZ,2);mouse_wheel=0;
        for(int i=0;i<3;++i)mouse_state.rgbButtons[i]=(buttons&(1u<<i))?112:0;mouse_state.rgbButtons[3]=0;
    }else{DIMOUSESTATE snapshot;poll_state(mouse_device,&snapshot,16);
        if(mouse_axis_mode==3){mouse_state.lX=snapshot.lX;mouse_state.lY=snapshot.lY;mouse_state.lZ=snapshot.lZ;}
        else{mouse_state.lX=add_mouse_delta(mouse_state.lX,snapshot.lX);mouse_state.lY=add_mouse_delta(mouse_state.lY,snapshot.lY);mouse_state.lZ=add_mouse_delta(mouse_state.lZ,snapshot.lZ);
            if(mouse_clip){mouse_state.lX=clip_axis(mouse_state.lX,0);mouse_state.lY=clip_axis(mouse_state.lY,1);mouse_state.lZ=clip_axis(mouse_state.lZ,2);}}
        for(int i=0;i<4;++i)mouse_state.rgbButtons[i]=snapshot.rgbButtons[i];}return &mouse_state;
    case 7:return reinterpret_cast<void*>(mouse_access);case 8:return reinterpret_cast<void*>(mouse_axis_mode);case 9:return reinterpret_cast<void*>(mouse_clip);
    default:kernel::error("[MOUSE] - Invalid getstate\n");return nullptr;}}

void mouse_bounds(int min_x,int min_y,int min_z,int max_x,int max_y,int max_z){
    mouse_minimum[0]=min_x;mouse_minimum[1]=min_y;mouse_minimum[2]=min_z;mouse_maximum[0]=max_x;mouse_maximum[1]=max_y;mouse_maximum[2]=max_z;
    window_input::bound_cursor(min_x,min_y,max_x,max_y);}
}
