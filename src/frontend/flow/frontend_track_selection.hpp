#pragma once
#include "frontend_scene_widgets.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/audio/music/music_clients.hpp"
namespace nfs::application::frontend {
void clear_menu_input_queue();
void restore_menu_rule_bindings(void *);
void set_menu_image_enabled(Dword, Dword);
void stop_track_description(void *);
void play_track_description(void *, Dword);
void track_purchase_error();
inline void *selected_track_record(TrackSelection *screen) {
    return game::runtime::at<void *>(screen->tracks, 12 + Dword(screen->track) * 4);
}
inline Dword selected_track_cost(TrackSelection *screen) {
    return game::runtime::at<Dword>(selected_track_record(screen),
                                   32 + Dword(screen->direction) * 4);
}
inline Widget *track_child(void *owner, const char *name, const void *type) {
    auto *child = static_cast<Widget *>(pending::find_child(owner, name));
    return child && game::runtime::invoke<Dword>(child, 2, type) ? child : nullptr;
}
} // namespace nfs::application::frontend
