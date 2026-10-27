#include "car_marker_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

void __fastcall draw_ai_cop_marker(void *entity, void *, Dword view_word, Dword progress) {
    void *view = reinterpret_cast<void *>(view_word);
    if (invoke<void *>(view, 2)) {
        if (Byte(progress))
            draw_cop_progress_marker(entity, view);
        else
            draw_vehicle_marker(entity, view, 0, 255, 0);
    }
}

void __fastcall draw_ai_racer_marker(void *entity, void *, Dword view_word, Dword) {
    void *view = reinterpret_cast<void *>(view_word);
    void *perpetrator = entities::interface_at(entity, 4);
    if (!at<void *>(perpetrator, 8)) {
        if (recently_stunned(entity, 0))
            draw_vehicle_marker(entity, view, 0, 0, 255);
        return;
    }
    if (at<Byte>(perpetrator, 25)) {
        if (at<Byte>(perpetrator, 24))
            draw_vehicle_marker(entity, view, 255, 0, 0);
        else
            draw_vehicle_marker(entity, view, 255, 128, 0);
    } else
        draw_vehicle_marker(entity, view, 255, 255, 0);
}

void __fastcall draw_player_marker(void *entity, void *, Dword view_word, Dword) {
    void *view = reinterpret_cast<void *>(view_word);
    const int frame = invoke<int>(entity, 50);
    const bool chased = at<void *>(entities::interface_at(entity, 4), 8) != nullptr;
    const Byte stunned = recently_stunned(entity, 0);
    if (frame < 1) {
        if (chased)
            draw_vehicle_marker(entity, view, 255, 0, 0);
        else if (stunned)
            draw_vehicle_marker(entity, view, 0, 0, 255);
        else
            draw_vehicle_marker(entity, view, 0, 255, 0);
    } else if (chased)
        draw_vehicle_marker(entity, view, 255, 255, 0);
    else
        draw_vehicle_marker(entity, view, 0, 0, 0);
}
} // namespace nfs::game::runtime::ai::cars
