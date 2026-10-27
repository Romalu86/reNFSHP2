#pragma once
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
namespace nfs::window_input {
using kernel::Dword;
using Byte=std::uint8_t;
using MessageCallback=Dword(__stdcall*)(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
struct MessageEntry { Dword message; MessageCallback callback; };
struct WindowState {
    Dword fields[3]; HWND handle, secondary; Dword hook_flags; LONG origin_x, origin_y;
    std::uint8_t owned; std::uint8_t padding[3];
};
static_assert(sizeof(WindowState)==36 && offsetof(WindowState,handle)==12);
extern WindowState window_state;
extern HWND& window; extern kernel::Lock message_lock;extern MessageEntry messages[128];extern Dword message_count;
extern Dword &hook_disabled;extern std::uint8_t &hook_enabled;extern HOOKPROC hook_procedure;extern FARPROC hook_get_values;
extern Dword hook_snapshot_valid;
extern Dword cursor_buttons;extern int cursor_x,cursor_y;
extern kernel::Lock keyboard_lock;extern Dword key_queue[96];extern int key_read,key_write,key_capacity;
HWND handle();Dword __stdcall register_message(Dword,MessageCallback);std::uint8_t initialize_hook();
void cursor_state(Dword*,LONG*,LONG*);
Dword read_key_queue();
extern std::uint8_t dispatch_enabled, application_active, window_active, window_initialized, cursor_lock_initialized;
extern std::uint8_t suppress_alt_keys;
extern Dword key_events_enabled, destroy_requested, window_class_users;
extern int cursor_min_x,cursor_min_y,cursor_max_x,cursor_max_y;
extern Byte* scan_remap;
extern Byte key_states[128];
using SystemKey = Dword(__cdecl*)(Dword,Dword);
extern SystemKey system_key;
extern HHOOK keyboard_hook, low_level_hook;
extern HINSTANCE window_instance;
extern const char* window_class_name;
using Redraw=void(__cdecl*)();
extern Redraw redraw;
void initialize_messages();void bind_window(HWND,HWND,Dword);void release_window();
std::uint8_t dispatch(HWND,Dword,WPARAM,LPARAM,LRESULT*);
bool post_message(Dword,LPARAM,WPARAM,int);
void key_queue_capacity(int);void enqueue_key(std::uint16_t,Byte);Dword navigation_key(Dword,Dword);
void position_cursor(int,int);void update_cursor(int,int,Dword);void bound_cursor(int,int,int,int);
void move_system_cursor(WindowState*,int,int);
void set_system_shortcuts(int);void restore_system_shortcuts();
LRESULT CALLBACK keyboard_filter(int,WPARAM,LPARAM);
LRESULT CALLBACK low_level_filter(int,WPARAM,LPARAM);
namespace handlers {
Dword __stdcall create(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall activate_application(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall activate_window(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall destroy(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall close(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall request_destroy(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall system_command(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall settings_change(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall moved(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall character(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall key_down(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall key_up(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall mouse_down(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall mouse_move(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall paint(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall erase_background(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
Dword __stdcall power(Dword,HWND,Dword,WPARAM,LPARAM,Dword);
}
}
