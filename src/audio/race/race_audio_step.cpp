#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/audio/backend/sound_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/audio/music/music_clients.hpp"
namespace nfs::game::runtime::pending {

void update_race_audio() {
    using namespace race_audio;
    using namespace race_audio::pending;
    if (!network::graphics_enabled)
        return;
    for (unsigned i = 0; i < 112; ++i) {
        if (event_flags[i] == 1)
            event_flags[i] = 2;
        if (event_flags[124 + i] == 1)
            event_flags[124 + i] = 2;
    }
    for (int listener = 0; listener < race_audio_listener_count; ++listener) {
        listener_index = listener;
        at<void *>(listener_state, 2792 + listener_index * 2760) =
            physics::local_car(system(68), listener);
        Dword *camera = invoke<Dword *>(tick_services[listener], 2);
        Dword position[3];
        camera_position(camera, position);
        for (unsigned axis = 0; axis < 3; ++axis)
            at<Dword>(listener_state, axis * 4) = position[axis];
        camera_orientation(camera, reinterpret_cast<float *>(listener_state + 12));
        const Dword phase = at<Dword>(camera, 72);
        if (phase == 9 || phase == 4 || phase == 6) {
            if (!at<Byte>(listener_state, 2860 + listener_index * 2760)) {
                sound::set_master_volume(120);
                if (music_mode == Dword(-3))
                    set_cd_audio_volume(0);
                else {
                    sound::lock();
                    if (phase == 9)
                        set_stream_volume(music_handle,
                                          group_volume(music_level, 5, 0));
                    else if (phase == 4)
                        fade_stream_volume(music_handle, 500,
                                           group_volume(music_level, 5, 0));
                    sound::unlock();
                }
                at<Byte>(listener_state, 2860 + listener_index * 2760) = 1;
            }
            const int selected_listener = listener_index;
            reset_listener(0);
            reset_listener(1);
            reset_environment();
            reset_engine_events();
            if (phase == 9 || phase == 4) {
                play_event((selected_listener ? 256 : 0) | 12, phase == 9 ? 7 : 0, 64,
                                    127, 0, 1);
                event_flags[12 + listener_index * 124] = 1;
            }
            if (phase == 6)
                update_finished_car(
                    at<void *>(listener_state, 2792 + listener_index * 2760),
                    listener_index);
        } else {
            if (at<Byte>(listener_state, 2860 + listener_index * 2760)) {
                if (music_mode == Dword(-3))
                    set_cd_audio_volume(group_volume(music_level, 5, 127));
                else {
                    sound::lock();
                    set_stream_volume(music_handle,
                                      group_volume(music_level, 5, 127));
                    sound::unlock();
                }
                at<Byte>(listener_state, 2860 + listener_index * 2760) = 0;
            }
            set_listener(listener_state, &listener_configuration);
            update_car_sounds();
            update_surfaces(4);
            car_channel =
                at<Dword>(listener_state, 104 + listener_index * 2760);
            for (unsigned offset = 0; offset < 112; offset += 28)
                update_wheel(listener_state + 108 + offset +
                                      listener_index * 2760);
            update_nearest();
        }
        for (int i = 0;
             i < at<int>(listener_state, 580 + listener_index * 2760); ++i)
            update_emitter(listener_state + 584 +
                                    listener_index * 2760 + i * 48);
        retire_unused_channels();
        for (int i = 0;
             i < at<int>(listener_state, 384 + listener_index * 2760); ++i)
            update_emitter_channel(listener_state + 2120 +
                                   listener_index * 2760 + i * 56);
        update_engines();
        update_impacts();
        at<Dword>(listener_state, 2832 + listener_index * 2760) = 10000;
    }
    play_due_events();
    game::audio::update_volume_fades();
    for (Dword i = 0; i < 112; ++i) {
        if (event_flags[i] == 2) {
            stop_event(i);
            event_flags[i] = 0;
        }
        if (event_flags[124 + i] == 2) {
            stop_event(i | 256);
            event_flags[124 + i] = 0;
        }
    }
}
} // namespace nfs::game::runtime::pending
