#pragma once
#include "nfs/audio/streams/audio_streams.hpp"
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
namespace nfs::game::audio {
using kernel::Dword;
using Byte = std::uint8_t;
struct TrackInfo {
    char artist[260], title[260], album[260];
};
struct Library {
    char current_path[260];
    TrackInfo tracks[50];
    char title[260], artist[260], album[260];
    int fade_setting, playing, current_track, selection;
    Dword stream;
    int track_count, instrumental_count, vocal_count, first_instrumental;
    Dword reserved_state, reserved_clock, remaining_ms, notify;
    Dword played[50];
    int custom_playlist;
    char directory[260], paths[50][260];
};
struct MusicPlayer {
    const lifecycle::SystemMethods *methods;
    Dword block;
    void *buffer;
    int last_volume;
};
struct CarSound {
    int volume, bend_range, bass_add, inside, outside;
    char bank[16], engine[8], exhaust[8];
};
struct EventChannel {
    Dword voice, event;
    Byte bank, reserved[3];
    int volume;
    Dword group;
};
struct BankBinding {
    int handle;
    Dword reserved;
    void *data;
    int vehicle_kind;
};
struct WaveProcessor {
    Dword kind, reserved;
    void *channels[6];
};
static_assert(sizeof(TrackInfo) == 780 && sizeof(Library) == 53556);
static_assert(offsetof(Library, tracks) == 260 && offsetof(Library, playing) == 40044);
static_assert(offsetof(Library, played) == 40092 && offsetof(Library, paths) == 40556);
static_assert(sizeof(MusicPlayer) == 16 && sizeof(CarSound) == 52);
static_assert(sizeof(EventChannel) == 20 && sizeof(BankBinding) == 16 &&
              sizeof(WaveProcessor) == 32);
namespace state {
namespace layout {
// Byte offsets into the existing audio state arrays.
// Configuration storage.
inline constexpr int audio_device_enabled = 0;
inline constexpr int audio_enabled = 4;
inline constexpr int music_muted = 16;
inline constexpr int car_sounds = 20;
inline constexpr int horn_names = 1320;
inline constexpr int player_volume = 1576;
inline constexpr int second_player_volume = 1580;
inline constexpr int opponent_volume = 1584;
inline constexpr int track_volume = 1588;
inline constexpr int car_collision_volume = 1592;
inline constexpr int boom_collision_volume = 1596;
inline constexpr int boom_loop_volume = 1600;
inline constexpr int boom_oneshot_volume = 1604;
inline constexpr int player_skid_volume = 1608;
inline constexpr int music_volume = 1612;
inline constexpr int speech_volume = 1616;
inline constexpr int car_volume_limit = 1620;
inline constexpr int traffic_volume = 1624;
inline constexpr int baseline_player_volume = 1628;
inline constexpr int baseline_second_player_volume = 1632;
inline constexpr int baseline_opponent_volume = 1636;
inline constexpr int baseline_track_volume = 1640;
inline constexpr int baseline_car_collision_volume = 1644;
inline constexpr int baseline_boom_collision_volume = 1648;
inline constexpr int baseline_boom_loop_volume = 1652;
inline constexpr int baseline_boom_oneshot_volume = 1656;
inline constexpr int baseline_player_skid_volume = 1660;
inline constexpr int baseline_music_volume = 1664;
inline constexpr int baseline_speech_volume = 1668;
inline constexpr int baseline_car_volume_limit = 1672;
inline constexpr int baseline_traffic_volume = 1676;
inline constexpr int reverb_preset = 1680;
// Option storage.
inline constexpr int playlist_count = 0;
inline constexpr int cd_open = 4;
inline constexpr int suppress_cd_poll = 8;
inline constexpr int skip_cd_ready_wait = 12;
inline constexpr int cd_track_count = 24;
inline constexpr int cd_current_track = 28;
inline constexpr int music_source = 32;
inline constexpr int music_ducked = 144;
inline constexpr int speech_stream_active = 148;
inline constexpr int player_engine_level = 156;
inline constexpr int second_player_engine_level = 160;
inline constexpr int player_attenuation_steps = 164;
inline constexpr int max_cars = 168;
inline constexpr int engine_type = 172;
inline constexpr int last_announced_lap = 176;
inline constexpr int car_name = 184;
// Spectrum storage.
inline constexpr int spectrum_input_cursor = 1024;
inline constexpr int spectrum_random_value = 17416;
inline constexpr int spectrum_imaginary_output = 50192;
inline constexpr int spectrum_imaginary_input = 82960;
inline constexpr int spectrum_real_output = 115728;
inline constexpr int spectrum_real_input = 148496;
// World storage.
inline constexpr int volume_fades = 5872;
inline constexpr int volume_fade_delay = 6880;
// Track storage.
inline constexpr int wind_sections = 0;
inline constexpr int sound_point_count = 80;
inline constexpr int actor_sound_count = 84;
inline constexpr int actor_sound_event_count = 88;
inline constexpr int emitter_count = 92;
inline constexpr int music_trigger_count = 96;
inline constexpr int trigger_event_count = 100;
inline constexpr int actor_sounds = 104;
inline constexpr int actor_sound_events = 828;
inline constexpr int sound_points = 1544;
inline constexpr int music_triggers = 9224;
inline constexpr int trigger_events = 9264;
inline constexpr int emitter_info = 9300;
} // namespace layout
extern Byte audio_settings[208], audio_heap[409600], client_configuration[2492];
extern Byte music_library[53556], client_options[216], wave_processors[64];
extern Byte event_channels[4960], bank_storage[784], channel_variation[248],
    effects_storage[64];
extern Byte spectrum_storage[246816];
extern Byte audio_world[6920];
extern Byte track_data[12320];
extern Byte engine_storage[256];
extern Dword waveform_buffer;
extern Dword &disable_music, &disable_audio, &quick_fade;
inline Library &library() {
    return *reinterpret_cast<Library *>(music_library);
}
inline EventChannel *channels() {
    return reinterpret_cast<EventChannel *>(event_channels);
}
inline BankBinding *banks() {
    return reinterpret_cast<BankBinding *>(bank_storage);
}
template <class T> T &configuration(unsigned byte_offset) {
    return sound::field<T>(client_configuration, byte_offset);
}
template <class T> T &option(unsigned byte_offset) {
    return sound::field<T>(client_options, byte_offset);
}
template <class T> T &spectrum(unsigned byte_offset) {
    return sound::field<T>(spectrum_storage, byte_offset);
}
template <class T> T &world(unsigned byte_offset) {
    return sound::field<T>(audio_world, byte_offset);
}
template <class T> T &track(unsigned byte_offset) {
    return sound::field<T>(track_data, byte_offset);
}
inline const char *path(unsigned offset) {
    return reinterpret_cast<const char *>(lifecycle::manager->slots[1]) + offset;
}
} // namespace state
int choose_track();
void read_catalog(int);
void initialize_music_stream(MusicPlayer *);
void begin_track(MusicPlayer *, int);
void update_music(MusicPlayer *, int);
void start_frontend_music(MusicPlayer *);
void restore_music_volume();
int has_multichannel_audio();
void read_volumes();
void initialize_configuration();
void apply_event_volumes();
void load_bank(const char *, int, Dword);
int load_archive_bank(const char *, const char *, int, Dword);
void release_frontend_bank();
void load_frontend_bank();
void load_reverb_resources();
void play_frontend_event(int, Dword);
Dword play_bank_event(int, int, std::uint16_t, Byte, Byte);
void stop_game_channels();
void stop_race_channels();
void mute_game_audio();
void restore_game_volume();
void reset_game_audio();
void release_game_audio();
void release_race_banks();
void reset_engine_state();
void initialize_audio_device();
void poll_music(Dword);
int wait_for_cd();
void read_cd_tracks();
int next_cd_track();
void play_cd_track(int);
void stop_cd_track();
void update_cd_music();
void select_music(MusicPlayer *, int);
void fade_out_music(MusicPlayer *, Dword);
void initialize_race_resources();
void initialize_race_listeners();
void load_race_banks_if_ready();
Byte car_environment(void *);
void apply_reverb_preset(int);
void reset_audio_world();
void reset_car_audio(void *);
void load_track_audio_configuration();
void initialize_volume_fades();
void set_volume_fade(int, int, float);
void update_volume_fades();
void construct_waveform(void *, Dword, int);
void destroy_waveform(void *);
int process_waveform(void *, int, void *, void *, Byte);
namespace spectrum {
bool power_of_two(Dword);
Dword index_bits(Dword);
Dword reverse_bits(Dword, Dword);
long double frequency(Dword, Dword);
long double sine_window(float);
void reset(Dword, Dword, int);
void transform(Dword, int, const float *, const float *, float *, float *);
void magnitudes(int *, const float *, const float *, int);
} // namespace spectrum
namespace pending {
void release_track_audio();
void load_race_banks();
                        // owners remain open.
} // namespace pending
} // namespace nfs::game::audio
namespace nfs::sound {
int find_bank_slot();
int initialize_bank(int *, void *);
int release_bank(int);
int bank_bytes(int);
int relocate_bank(void *, int);
int bind_bank_layer(void *, void *);
int release_bank_layer(void *);
} // namespace nfs::sound
