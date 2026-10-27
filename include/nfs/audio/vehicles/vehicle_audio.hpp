#pragma once
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::kernel::numeric { class Extended; }
namespace nfs::game::audio::vehicle {
using runtime::at;
using runtime::bytes;
struct EngineOwner {
    alignas(4) Byte bytes[444];
};
struct EngineBank {
    int handle, exhaust;
    void *data;
    int pitch;
};
struct Progress {
    int total, completed;
    Progress *next, *previous;
};
static_assert(sizeof(EngineOwner) == 444 && sizeof(EngineBank) == 16 && sizeof(Progress) == 16);
extern Byte engine_owners[816], engine_curves[392];
extern Byte &siren_rising;
extern Progress *progress_head;
inline EngineOwner *&owner(int listener) {
    return at<EngineOwner *>(engine_owners, 808 + listener * 4);
}
inline EngineBank &bank(int listener, int patch) {
    return at<EngineBank>(engine_owners, listener * 384 + patch * 16);
}
inline Byte *&table(unsigned va, int listener) {
    return at<Byte *>(engine_owners, va - 7218112 + listener * 4);
}
inline int wrapped_product(int left, int right) {
    return int(Dword(left) * Dword(right));
}
inline int cutoff() {
    return at<Dword>(runtime::system(72), 140) ? 10000 : 5625;
}
inline int attenuate(int distance, int range, int gain) {
    return distance < range ? wrapped_product(int(Dword(range) - Dword(distance)), gain) / range : 0;
}
inline int fixed_pitch(int scale, int pitch) {
    return wrapped_product(scale, pitch) / 65536;
}
void load_patch(const char *, const char *, const char *, int, int, int);
void load_patch_group(const char *, const char *, int, int);
void bind_engine_tables(int);
int initialize_engine_owner(int, int);
void release_engine_owners();
void mix_engine_owner(int);
void mix_engine_owners();
void set_engine_parameters(int, std::uint16_t, int, int, int, std::uint16_t, int, void *, int);
int normalized_rpm(void *);
int engine_rpm(void *);
int clamp_value(int, int, int);
int selected_camera_mode(void *);
Byte car_audio_suppressed(void *);
Progress *construct_progress(Progress *, int);
void advance_progress(Progress *);
void destroy_progress(Progress *);
kernel::numeric::Extended progress_weight(const Progress *);
kernel::numeric::Extended progress_fraction(const Progress *);
kernel::numeric::Extended combined_progress();
void select_surface_sound(Dword, int, Dword, int, int, Dword);
void submit_distant_engine(int, int, Dword, int);
void stop_distant_engine();
void submit_wheel_sound(Dword, Dword, int, int, int, Dword, int);
void submit_tyre_sound(Dword, Dword, int, int, int, Dword, int);
void submit_surface_sound(Dword, int, Dword, int, int, Dword);
void update_player_engine(void *, int, int, Dword, int, int);
void update_player_events(void *, int);
void update_wind(void *, int);
void update_track_emitters(void *);
namespace pending {
const char *loading_widget_type();
Dword create_speech_event(Dword, Dword, Dword);
void flush_speech_events(Dword);
extern void(__cdecl *&speech_sink)(Dword, int, ...);
} // namespace pending
void request_lap_announcement(void *, int);
} // namespace nfs::game::audio::vehicle
namespace nfs::gui {
extern Object *engine_object;
Dword tick_root(void *);
} // namespace nfs::gui
