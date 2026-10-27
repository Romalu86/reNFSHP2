#include "nfs/input/window/window_input.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include <cstdlib>
namespace nfs::window_input {

HWND handle(){return window_state.secondary;}
// Signed subtraction, including the removal sentinel.
static int compare_message(const void* left,const void* right){return static_cast<int>(static_cast<const MessageEntry*>(left)->message-static_cast<const MessageEntry*>(right)->message);}
// Lock initialization belongs to the still-open window lifecycle.
Dword __stdcall register_message(Dword message,MessageCallback callback){kernel::lock_enter(&message_lock);MessageEntry key{message,nullptr};
    auto* entry=static_cast<MessageEntry*>(std::bsearch(&key,messages,message_count,8,compare_message));
    if(!entry){if(callback&&message_count<128){messages[message_count++]={message,callback};std::qsort(messages,message_count,8,compare_message);}}
    else if(!callback){entry->message=2147483647;std::qsort(messages,message_count,8,compare_message);--message_count;}
    else entry->callback=callback;kernel::lock_leave(&message_lock);return 1;}

void cursor_state(Dword* buttons,LONG* x,LONG* y){if(buttons)*buttons=cursor_buttons;if(x)*x=cursor_x;if(y)*y=cursor_y;}
// The window lifecycle initializes the lock and capacity.
Dword read_key_queue(){Dword value=0;int scan=0;kernel::lock_enter(&keyboard_lock);
    while(key_read!=key_write&&!value){Dword event=key_queue[key_read];value=static_cast<Dword>(static_cast<std::int16_t>(event>>16));scan=static_cast<std::int8_t>(event);
        if(!value)value=input::translate_key(scan);key_read=(key_read+1)%key_capacity;}
    kernel::lock_leave(&keyboard_lock);return (static_cast<Dword>(scan)<<16)|value;}
 
std::uint8_t initialize_hook(){if(hook_disabled)return 1;
    lifecycle::windows_hook_module=LoadLibraryA("Win2kHook.dll");if(lifecycle::windows_hook_module){
        hook_procedure=reinterpret_cast<HOOKPROC>(GetProcAddress(lifecycle::windows_hook_module,"_LowLevelKeyboardProc@12"));
        auto set_handle=reinterpret_cast<void(__cdecl*)(HHOOK)>(GetProcAddress(lifecycle::windows_hook_module,"SetHookHandle"));
        hook_get_values=GetProcAddress(lifecycle::windows_hook_module,"GetValues");
        if(hook_procedure&&set_handle&&hook_get_values){if(!hook_enabled)return 1;
            lifecycle::windows_hook=SetWindowsHookExA(13,hook_procedure,lifecycle::windows_hook_module,0);set_handle(lifecycle::windows_hook);
            lifecycle::windows_hook_install_error=GetLastError();return lifecycle::windows_hook!=nullptr;}}
    lifecycle::windows_hook_install_error=GetLastError();return 0;}
}
