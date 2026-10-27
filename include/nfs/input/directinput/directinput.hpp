#pragma once
#include "nfs/core/types/kernel.hpp"
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 1792
#endif
#include <dinput.h>
namespace nfs::input {
using kernel::Dword;
struct DeviceSlot { IDirectInputDevice2A* device; Dword flags,type; std::uint8_t data[256]; };
struct DeviceAbout { Dword type,flags,axes,buttons,povs; char instance[260],product[260]; };
struct EffectSlot { IDirectInputEffect* effect; int device_index; Dword fields[35]; };
static_assert(sizeof(DeviceSlot)==268 && sizeof(DeviceAbout)==540 && sizeof(EffectSlot)==148);
extern DeviceSlot devices[32]; extern int device_count; extern IDirectInputA* devices_interface;
extern IDirectInputA* keyboard_interface; extern IDirectInputDevice2A* keyboard_device;
extern IDirectInputA* mouse_interface; extern IDirectInputDevice2A* mouse_device;
extern std::uint8_t keyboard_state[256]; extern DeviceAbout keyboard_about,mouse_about;
extern DeviceAbout device_about;
void* device_get(int,Dword);Dword device_axis_mode(IDirectInputDevice2A*);
extern DIMOUSESTATE mouse_state; extern int mouse_minimum[3],mouse_maximum[3],mouse_wheel;
extern int mouse_access,mouse_axis_mode,mouse_clip,mouse_registered,mouse_exit_registered;
extern int keyboard_active,keyboard_exit_registered; extern bool caps_lock;
using KeyReader=Dword(__cdecl*)(); extern KeyReader key_reader;
extern const GUID keyboard_guid,mouse_guid,device2_guid;
extern const DIDATAFORMAT keyboard_format,mouse_format,joystick_format;
// DIEFFECT descriptors, COM pointers and owner indices share a 148-byte stride. Initialization
// state and keyboard backup share the same storage.
EffectSlot* effect_slots(); Dword& effects_initialized(); std::uint8_t* state_backup();
std::uint8_t* effect_record_data();
void stop_effect(int);Dword& effect_stop_state(int);
HRESULT start_effect_with_retry(IDirectInputDevice2A *, IDirectInputEffect *, Dword, Dword);
void start_effect(int);
Dword infinite_effect_active(int);
std::uint8_t update_effect(int, int, const void *, Dword *, const int *, const int *, const int *, const int *);
HRESULT poll_state(IDirectInputDevice2A*,void*,Dword);
HRESULT describe_device(IDirectInputDevice2A*,DeviceAbout*);
bool send_feedback_command(IDirectInputDevice2A*,Dword);
void effects_initialize();void effects_shutdown();bool reset_effects(int);void disable_feedback(int);
Dword devices_initialize();void devices_shutdown();BOOL CALLBACK enumerate_device(const DIDEVICEINSTANCEA*,void*);
void keyboard_initialize();void keyboard_release();void keyboard_shutdown();void* keyboard_get(int);
void mouse_initialize();void mouse_release();void mouse_shutdown();bool mouse_set(int,int);void* mouse_get(Dword);
void mouse_bounds(int,int,int,int,int,int);Dword translate_key(int);Dword scan_code(Dword);
Dword translate_state(const std::uint8_t*,std::uint8_t,int);
}
