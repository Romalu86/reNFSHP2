#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/console/console.hpp"
#include "nfs/graphics/backend/eagl_state.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/network/session/network_virtual.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world {
namespace {
struct ProgressScope {
    audio::vehicle::Progress progress;
    explicit ProgressScope(int total) {
        audio::vehicle::construct_progress(&progress, total);
    }
    ~ProgressScope() {
        audio::vehicle::destroy_progress(&progress);
    }
    void advance() {
        audio::vehicle::advance_progress(&progress);
    }
};
} // namespace

void initialize_track_catalog(void *owner) {
    void *rules = subsystem(72);
    char path[260], section[80], key[80], text[80];
    std::sprintf(path, "%s%s", static_cast<const char *>(subsystem(4)) + 524, "Tracks.ini");
    ini::Reader reader;
    ini::reader_construct(&reader, path, 0);
    try {
        at<Dword>(owner, 4) = 0;
        std::sprintf(section, "track%d", 0);
        while (ini::get_string(&reader, section, "name", text, 79)) {
            int index = at<int>(owner, 4);
            void *entry = lifecycle::allocate(52, "TrackDataEntry", lifecycle::level_pool);
            at<void *>(owner, 12 + index * 4) = entry;
            std::strcpy(static_cast<char *>(entry), text);
            Dword token = 0;
            text[0] = 0;
            ini::get_string(&reader, section, "displayName", text, 79);
            pending::resolve_text_token(text, &token);
            at<Dword>(entry, 12) = token;
            int value = 0;
            ini::get_int(&reader, section, "id", &value);
            at<int>(entry, 20) = value;
            value = 0;
            ini::get_int(&reader, section, "loop", &value);
            at<Byte>(entry, 24) = value != 0;
            for (int mode = 0; mode < 4; ++mode) {
                int price = 0;
                std::sprintf(key, "m_price[%d]", mode);
                ini::get_int(&reader, section, key, &price);
                at<int>(entry, 32 + mode * 4) = price;
                at<Byte>(entry, 25 + mode) = price == 0;
                if (!price && index < 12) {
                    Byte *unlock = offset(rules, 9950 + index * 4 + mode);
                    if (unlock < offset(rules, 9998))
                        *unlock = 1;
                }
            }
            text[0] = 0;
            ini::get_string(&reader, section, "trackDescrip", text, 79);
            token = Dword(value);
            pending::resolve_text_token(text, &token);
            at<Dword>(entry, 16) = token;
            float length = 0;
            ini::get_float(&reader, section, "length", &length);
            at<float>(entry, 48) = length;
            at<int>(owner, 4) = index + 1;
            std::sprintf(section, "track%d", index + 1);
        }
        void *current = subsystem(40);
        at<Dword>(current, 8) = 0;
        std::strncpy(reinterpret_cast<char *>(offset(current, 140)),
                     at<const char *>(current, 12), 12);
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}

void load_track_manager_level(void *) {
    ProgressScope progress(4);
    void *memory = lifecycle::allocate(44, "GOBManager", lifecycle::track_pool);
    runtime::track_collision =
        memory ? static_cast<Dword *>(construct_gob_database(memory)) : nullptr;
    progress.advance();
    pending::initialize_track_objects_entry();
    progress.advance();
    pending::initialize_track_collision_grid();
    progress.advance();
    load_gob_database(runtime::track_collision);
    progress.advance();
    finish_track_loading();
}

void unload_track_manager_level(void *) {
    kernel::print::check_console_write("track manager: start level deinit\n");
    pending::release_track_sky_entry();
    kernel::print::check_console_write("track manager: boom deinit\n");
    pending::release_track_objects_entry();
    kernel::print::check_console_write("track manager: sky deinit\n");
    if (runtime::track_collision) {
        kernel::print::check_console_write("track manager: delete gGOBDatabase\n");
        void *database = runtime::track_collision;
        if (database) {
            destroy_gob_database(database);
            kernel::memory_free(database);
        }
        runtime::track_collision = nullptr;
    }
    if (pending::gob_definitions) {
        kernel::print::check_console_write("track manager: delete gGOBDefs\n");
        void *definitions = pending::gob_definitions;
        if (definitions) {
            destroy_gob_definitions(definitions);
            kernel::memory_free(definitions);
        }
        pending::gob_definitions = nullptr;
    }
    kernel::print::check_console_write("track manager: end level deinit\n");
}

void load_track_level(void *owner) {
    ProgressScope overall(2);
    graphics::state::set_texture_filter_override(at<int>(subsystem(72), 2380) < 3 ? 2 : 3);
    pending::read_track_layout(owner);
    at<Dword>(owner, 48) = at<Dword>(owner, 16);
    at<Dword>(owner, 52) = at<Dword>(owner, 20);
    at<Byte>(owner, 64) = at<Byte>(owner, 65) = 1;
    at<Dword>(owner, 68) = at<Dword>(owner, 72) = 1;
    pending::track_load_begin =
        pending::legacy_clock_entry() - runtime::ai::lifecycle::pending::startup_ticks;
    for (unsigned index = 0; index < 4; ++index) {
        unsigned slot = 21672 + index * 8;
        if (at<int>(subsystem(72), 176) == 1) {
            at<void *>(owner, slot) = kernel::memory_allocate("Compartment buffer", 307200,
                                                              lifecycle::track_pool | 1024);
            at<void *>(owner, slot + 4) = nullptr;
        } else {
            at<void *>(owner, slot) =
                kernel::memory_allocate("Compartment buffer", 0, lifecycle::track_pool | 1024);
            at<void *>(owner, slot + 4) =
                kernel::memory_allocate("Compartment buffer", 0, lifecycle::track_pool | 1024);
        }
    }
    pending::load_compartment_directory(offset(owner, 76));
    overall.advance();
    {
        ProgressScope compartments(at<int>(owner, 4));
        for (int index = 0; index < at<int>(owner, 4); ++index) {
            pending::load_compartment(offset(owner, 168 + index * 336));
            compartments.advance();
        }
    }
    overall.advance();
    at<Byte>(owner, 8) = 0;
    invoke(owner, 5);
    at<Byte>(owner, 8) = 1;
    pending::track_load_end =
        pending::legacy_clock_entry() - runtime::ai::lifecycle::pending::startup_ticks;
    at<Dword>(owner, 12) = 0;
}

void unload_track_level(void *owner) {
    for (int player = 0; player < at<int>(subsystem(72), 176); ++player) {
        Dword &ready = at<Dword>(owner, 68 + player * 4);
        while (!ready) {
            ready = compartment_ready(owner, player);
            kernel::cooperate(0);
        }
    }
    pending::unload_compartment_directory_entry(offset(owner, 76));
    for (int index = 0; index < at<int>(owner, 4); ++index) {
        runtime::track::pending::reset_compartment(
            reinterpret_cast<Dword *>(offset(owner, 168 + index * 336)));
        runtime::track::pending::reset_compartment(
            reinterpret_cast<Dword *>(offset(owner, 336 + index * 336)));
    }
    for (unsigned index = 0; index < 4; ++index)
        for (unsigned bank = 0; bank < 2; ++bank) {
            void *&buffer = at<void *>(owner, 21672 + index * 8 + bank * 4);
            if (buffer) {
                kernel::memory_free(buffer);
                buffer = nullptr;
            }
        }
    for (int index = 0; index < at<int>(owner, 4); ++index)
        pending::release_compartment_entry(offset(owner, 168 + index * 336));
}

void reset_track_level(void *owner) {
    at<Byte>(owner, 21752) = 0;
    for (int player = 0; player < at<int>(subsystem(72), 176); ++player) {
        Dword &ready = at<Dword>(owner, 68 + player * 4);
        while (!ready) {
            ready = compartment_ready(owner, player);
            kernel::cooperate(0);
        }
    }
    if (!at<Byte>(owner, 8))
        for (unsigned index = 0; index < 64; ++index) {
            runtime::track::pending::reset_compartment(
                reinterpret_cast<Dword *>(offset(owner, 168 + index * 336)));
            runtime::track::pending::reset_compartment(
                reinterpret_cast<Dword *>(offset(owner, 336 + index * 336)));
        }
    const int players = at<int>(subsystem(72), 176);
    for (int player = 0; player < players; ++player) {
        at<Dword>(owner, 48 + player * 4) = at<Dword>(owner, 16 + player * 4);
        at<Byte>(owner, 64 + player) = 1;
        at<Dword>(owner, 68 + player * 4) = 1;
    }
    pending::initialize_track_positions(owner);
    pending::bind_track_texture_state_entry(owner);
}
} // namespace nfs::game::world
