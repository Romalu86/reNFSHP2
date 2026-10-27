#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_menu_pages.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/gui/events/gui_tasks.hpp"
#include <cstdio>
#include <cstdlib>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;
using namespace game::audio;
static Dword preview_race_music;
static family::SliderBar *audio_slider(void *page, const char *name) {
    auto *widget = static_cast<Widget *>(pending::find_child(page, name));
    return widget && invoke<Dword>(widget, 2, family::slider_bar_class_key)
               ? reinterpret_cast<family::SliderBar *>(widget)
               : nullptr;
}
static void append_music_choice(ProfileToggle *toggle, const gui::String *text) {
    family::ListItem item;
    construct_toggle_item(&item, text, 0);
    try {
        invoke(&toggle->items, 8, &item);
    } catch (...) {
        gui::string_destruct(&item.tooltip);
        gui::string_destruct(&item.text);
        throw;
    }
    gui::string_destruct(&item.tooltip);
    gui::string_destruct(&item.text);
}

static void change_audio_mode() {
    const int playlist = state::library().custom_playlist;
    if (state::option<int>(audio_layout::music_source) == -4)
        fade_out_music(reinterpret_cast<MusicPlayer *>(system(80)), 500);
    else {
        stop_cd_track();
        gui::unregister_task(poll_music);
    }
    release_frontend_bank();
    invoke(system(84), 7);
    invoke(system(80), 2);
    invoke(system(80), 1);
    state::library().custom_playlist = playlist;
    read_catalog(playlist);
    invoke(system(84), 6);
    invoke(system(80), 6);
    if (state::option<int>(audio_layout::music_source) == -4)
        select_music(reinterpret_cast<MusicPlayer *>(system(80)), -1);
    else {
        state::library().selection = -3;
        play_cd_track(next_cd_track());
        gui::register_task(poll_music);
    }
}

void __fastcall initialize_audio_page(AudioPage *page, void *) {
    gui::widgets::pending::request_general_layout(&page->base, nullptr);
    page->menu_music = audio_slider(page, "<MENU_MUSIC_VOLUME>");
    page->race_music = audio_slider(page, "<RACE_MUSIC_VOLUME>");
    page->engine = audio_slider(page, "<ENGINE_VOLUME>");
    page->voice = audio_slider(page, "<VOICE_VOLUME>");
    page->effects = audio_slider(page, "<SFX_VOLUME>");
    page->mode_widget = family::named_child_id(&page->base.base, "<AUDIO_MODE>");
    for (unsigned index = 0; index < 5; ++index)
        page->initial_levels[index] = network::packets::rule_value(system(72), 48 + int(index));
    page->mode = int(network::packets::rule_value(system(72), 18));
    page->music_type = find_page_toggle(page, "<MUSIC_TYPE>");
    if (!page->music_type)
        return;
    if (!state::option<int>(audio_layout::playlist_count)) {
        set_enabled(reinterpret_cast<Widget *>(page->music_type), 0);
        pending::set_widget_visible(page->music_type, 0);
        return;
    }
    {
        pending::LocalFrontendString title;
        gui::string_reference(&title.value, 328);
        invoke(page->music_type, 7, &title.value);
    }
    {
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, 1538);
        append_music_choice(page->music_type, &text.value);
    }
    for (int index = 0; index < state::option<int>(audio_layout::playlist_count); ++index) {
        char filename[260], section[32], name[260];
        std::sprintf(filename, "%splaylist.ini", graphics::startup::configuration_path());
        std::sprintf(section, "playlist%d", index);
        ini::Reader reader;
        ini::reader_construct(&reader, filename, 0);
        try {
            ini::get_string(&reader, section, "name", name, 259);
            pending::LocalFrontendString text(name);
            append_music_choice(page->music_type, &text.value);
        } catch (...) {
            ini::reader_clear(&reader);
            throw;
        }
        ini::reader_clear(&reader);
    }
    profiles::select_profile_row(page->music_type, Dword(state::library().custom_playlist));
    page->playlist = state::library().custom_playlist;
}

void __fastcall release_audio_page(AudioPage *page, void *) {
    if (!page->suppress_release_fade && state::configuration<Dword>(audio_layout::audio_enabled)) {
        const Dword volume = Dword(
            game::runtime::race_audio::group_volume(state::configuration<Dword>(audio_layout::music_volume), 4, 127));
        if (state::option<int>(audio_layout::music_source) == -3)
            game::runtime::race_audio::pending::set_cd_audio_volume(volume);
        else {
            sound::lock();
            game::runtime::race_audio::fade_stream_volume(state::library().stream, 500, volume);
            sound::unlock();
        }
    }
    clear_layout_request(&page->base.base, nullptr);
}

void __fastcall update_audio_page(AudioPage *page, void *, Dword) {
    const Dword volume = Dword(game::runtime::race_audio::group_volume(
        state::configuration<Dword>(audio_layout::music_volume), preview_race_music ? 5 : 4, 127));
    if (!state::configuration<Dword>(audio_layout::audio_enabled))
        return;
    if (state::option<int>(audio_layout::music_source) == -3)
        game::runtime::race_audio::pending::set_cd_audio_volume(volume);
    else {
        sound::lock();
        game::runtime::race_audio::set_stream_volume(state::library().stream, volume);
        sound::unlock();
    }
    auto *toggle = page->music_type;
    if (toggle && family::selected_list_row(&toggle->items) != page->playlist &&
        state::option<int>(audio_layout::playlist_count)) {
        const int playlist = family::selected_list_row(&toggle->items);
        state::library().custom_playlist = playlist;
        page->playlist = playlist;
        profiles::select_profile_row(toggle, Dword(playlist));
        fade_out_music(reinterpret_cast<MusicPlayer *>(system(80)), 500);
        read_catalog(state::library().custom_playlist);
        select_music(reinterpret_cast<MusicPlayer *>(system(80)), -1);
    }
    if (int(network::packets::rule_value(system(72), 18)) != page->mode) {
        page->mode = int(network::packets::rule_value(system(72), 18));
        change_audio_mode();
    }
}

Dword __fastcall audio_page_key(AudioPage *page, void *, Dword key, Dword modifiers) {
    return gui::widgets::pending::general_key(&page->base, nullptr, key, modifiers);
}

static Dword __cdecl preview_race(void *, Dword, Dword, Dword) {
    preview_race_music = 1;
    return 0;
}

static Dword __cdecl preview_menu(void *, Dword, Dword, Dword) {
    preview_race_music = 0;
    return 0;
}
static CallbackRecord audio_callbacks[2];

static void unregister_audio_callbacks() {
    for (unsigned index = 2; index; --index)
        unregister_callback(audio_callbacks + index - 1);
}

static void register_audio_callbacks() {
    register_callback(audio_callbacks, general_class_name, "GEVENT_FOCUSSET", 14,
                      "AudioScreen_SetFocusRace", preview_race, 0);
    register_callback(audio_callbacks + 1, general_class_name, "GEVENT_FOCUSSET", 14,
                      "AudioScreen_SetFocusMenu", preview_menu, 0);
    std::atexit(unregister_audio_callbacks);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_audio_callbacks, ".CRT$XCU_NFS_00690044",
                       register_audio_callbacks);
} // namespace
} // namespace nfs::application::frontend
