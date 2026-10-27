#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstring>
namespace nfs::graphics::backend {

void present(Dword* object){Sleep(0);const int result=com(backend_device,60,Dword(0),Dword(0),Dword(0),Dword(0));if(result==static_cast<int>(2289436776u)&&!recover_device(object))return;advance_frame(object+2);++state::render_frame_serial;}

void drain_frames(Dword* object){while(owner(object)[136/4]){com(backend_device,60,Dword(0),Dword(0),Dword(0),Dword(0));advance_frame(owner(object)+2);}}
// Authorized display compatibility: cooperative lost-device
// waiting, preserved quit delivery, and no resource rebinding after failed Reset.
bool recover_device(Dword *object) {
    constexpr int device_lost = static_cast<int>(2289436776u);
    constexpr int device_not_reset = static_cast<int>(2289436777u);
    if (!backend_device)
        return false;
    int result = com(backend_device, 12);
    while (result == device_lost) {
        Dword callback = owner(object)[188 / 4];
        if (callback)
            reinterpret_cast<void(__cdecl *)()>(callback)();
        if (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                PostQuitMessage(static_cast<int>(message.wParam));
                return false;
            }
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        // Same one-millisecond yield as context_default_error, including when
        // the application's suspension callback replaces that default callback.
        Sleep(1);
        if (!backend_device)
            return false;
        result = com(backend_device, 12);
    }
    if (!result)
        return true;
    stream_state = 4294967295u;
    if (result != device_not_reset || reset_device(object) < 0) {
        Dword callback = owner(object)[196 / 4];
        if (callback)
            reinterpret_cast<void(__cdecl *)()>(callback)();
        return false;
    }
    unresolved::buffer_bindings();
    unresolved::render_bindings();
    unresolved::vertex_state();
    initialize_texture_stages();
    unresolved::texture_state();
    state::render_frame_serial = 0;
    Dword callback = owner(object)[188 / 4];
    if (callback)
        reinterpret_cast<void(__cdecl *)()>(callback)();
    return true;
}
}
