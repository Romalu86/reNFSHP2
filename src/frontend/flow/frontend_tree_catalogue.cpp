#include "../screens/frontend_menu_roots.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::application::frontend {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;

TreeRace *initialize_tree_race(TreeRace *event) {
    event->bronze_branch = 0;
    event->trunk_unlocked = 1;
    event->type = event->opponents = event->car_class = event->skill = event->objective =
        event->cop_intensity = event->race_count = event->title = event->time_limit =
            event->roadblocks = event->helicopters = event->backup = event->traffic = 0;
    for (auto &track : event->tracks)
        for (int i = 6; i >= 0; --i)
            track[i] = 0;
    event->car_count = event->opponent_car_count = 0;
    event->reserved_238 = event->trunk = event->status = 0;
    event->processed = 0;
    std::memset(event->rewards, 0, 12);
    std::memset(event->branches, 0, 8);
    std::memset(event->cars, 0, 96);
    event->navigation[3] = event->navigation[2] = event->navigation[1] = event->navigation[0] = 0;
    for (unsigned i = 0; i < 3; ++i)
        for (unsigned kind = 0; kind < 6; ++kind)
            event->criteria[kind * 3 + i] = -1;
    return event;
}
// Consumes the section-name argument; native count parameter is unused.
void read_tree_text_list(gui::String section, Dword *output, int) {
    struct Owned {
        gui::String *s;
        ~Owned() {
            gui::string_destruct(s);
        }
    } owned{&section};
    alignas(4) std::uint8_t configuration[56];
    {
        pending::LocalFrontendString filename("lists.ini");
        gui::engine::pending::create_configuration(configuration, &filename.value);
    }
    try {
        gui::engine::pending::select_section(configuration, gui::string_ascii(&section));
        const Dword count = invoke<Dword>(configuration, 3);
        for (Dword index = 0; int(index) < int(count); ++index) {
            pending::LocalFrontendString name(invoke<const gui::String *>(configuration, 4, index));
            const char *text = gui::string_ascii(&name.value);
            if (!text)
                output[index] = 0;
            else {
                char token[136];
                std::sprintf(token, "%s", text);
                Dword reference = 0;
                game::world::pending::resolve_text_token(token, &reference);
                output[index] = reference;
            }
        }
    } catch (...) {
        gui::engine::pending::destroy_configuration(configuration);
        throw;
    }
    gui::engine::pending::destroy_configuration(configuration);
}

Dword tree_race_type(int index) {
    const int types[18] = {0, -1, 1, 2, 3, -1, 7, -1, 9,
                           10, -1, 12, 5, 8, -1, 11, -1, -1};
    const int type = types[index];
    return type == -1 ? 0 : Dword(type);
}

void load_tree_events(TreeScreen *screen, gui::String name) {
    struct Owned {
        gui::String *s;
        ~Owned() {
            gui::string_destruct(s);
        }
    } owned{&name};
    char filename[260];
    std::sprintf(filename, "%s%s.ini", bytes(system(4)) + 2344, gui::string_ascii(&name));
    if (!files::file::exists(filename))
        return;
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        for (unsigned event_index = 0; event_index < 33; ++event_index) {
            char section[80], key[80], value[80];
            std::sprintf(section, "event%d", event_index);
            int number = 0;
            if (!ini::get_int(&reader, section, "m_TrunkNode", &number))
                break;
            TreeRace &event = screen->races[event_index];
            event.trunk = Dword(number);
            event.bronze_branch = -1;
            for (unsigned branch = 0; branch < 2; ++branch) {
                std::sprintf(key, "m_BranchNodes[%d]", branch);
                value[0] = 0;
                ini::get_string(&reader, section, key, value, 79);
                if (std::sscanf(value, "%d", &number) == 1) {
                    event.branches[branch] = Dword(number);
                    if (std::strstr(value, "bronze"))
                        event.bronze_branch = number;
                }
            }
            for (unsigned direction = 0; direction < 4; ++direction) {
                number = 0;
                std::sprintf(key, "m_NavNode[%d]", direction);
                ini::get_int(&reader, section, key, &number);
                if (number < 0)
                    number = 0;
                else if (number > 32)
                    number = 32;
                event.navigation[direction] = Dword(number);
            }
            auto integer = [&](const char *field) {
                int result = 0;
                ini::get_int(&reader, section, field, &result);
                return Dword(result);
            };
            auto reference = [&](const char *field) {
                char token[80];
                token[0] = 0;
                ini::get_string(&reader, section, field, token, 79);
                Dword result = 0;
                game::world::pending::resolve_text_token(token, &result);
                return result;
            };
            event.objective = reference("m_Objective");
            event.title = reference("m_eventTitle");
            event.time_limit = integer("m_timeLimit");
            const int race_types[18] = {0,  -1, 1,  2, 3, -1, 7,  -1, 9,
                                        10, -1, 12, 5, 8, -1, 11, -1, -1};
            const int type = race_types[integer("m_raceType")];
            event.type = type == -1 ? 0 : Dword(type);
            event.skill = integer("m_OppSkill");
            event.opponents = integer("m_numOpps");
            event.car_class = integer("m_OppCarClass");
            event.opponent_car_count = int(integer("m_NumOppCarList"));
            void *catalogue = bytes(system(52)) + 4;
            for (int index = 0; index < event.opponent_car_count; ++index) {
                std::sprintf(key, "m_OppCarList[%d]", index);
                value[0] = 0;
                ini::get_string(&reader, section, key, value, 79);
                event.opponent_cars[index] = Dword(services::find_catalog_car(catalogue, value));
            }
            for (unsigned rank = 0; rank < 3; ++rank) {
                const char *fields[] = {"m_raceTime[%d]",   "m_lapTime[%d]",     "m_racePlace[%d]",
                                        "m_tournPlace[%d]", "m_tournPoints[%d]", "m_NumArrests[%d]",
                                        "m_reward[%d]"};
                const unsigned offsets[] = {32, 20, 44, 56, 68, 8, 80};
                for (unsigned field = 0; field < 7; ++field) {
                    number = 0;
                    std::sprintf(key, fields[field], rank);
                    if (ini::get_int(&reader, section, key, &number))
                        at<int>(&event, offsets[field] + rank * 4) = number;
                }
            }
            event.cop_intensity = integer("m_CopIntensity");
            event.roadblocks = integer("m_numRoadBlock");
            event.helicopters = integer("m_numCopter");
            event.backup = integer("m_numBackUp");
            event.car_count = int(integer("m_NumAvailableCars"));
            for (int index = 0; index < event.car_count; ++index) {
                std::sprintf(key, "m_AvailableCarList[%d]", index);
                ini::get_string(&reader, section, key, value, 79);
                event.cars[index] = Dword(services::find_catalog_car(catalogue, value));
            }
            event.traffic = integer("m_traffic");
            event.race_count = integer("m_numRaces");
            for (Dword track = 0; int(track) < int(event.race_count); ++track) {
                auto track_integer = [&](const char *field) {
                    std::sprintf(key, field, track + 1);
                    number = 0;
                    ini::get_int(&reader, section, key, &number);
                    return Dword(number);
                };
                event.tracks[track][3] = event.race_count;
                event.tracks[track][0] = track_integer("m_trackID[%d]");
                event.tracks[track][1] = track_integer("m_direction[%d]");
                event.tracks[track][2] = track_integer("m_numLaps[%d]");
                event.tracks[track][6] = track_integer("m_cops[%d]");
            }
        }
        const Dword mode = at<Dword>(system(72), 148);
        for (unsigned index = 0; index < 33; ++index) {
            const unsigned offset = mode == 2 ? 8180 : mode == 4 ? 28772 : 0;
            void *record = offset ? bytes(screen->preferences) + offset + index * 624 : nullptr;
            if (record) {
                screen->races[index].status = at<Dword>(record, 0);
                if (mode == 4)
                    screen->races[index].trunk_unlocked = 0;
            }
        }
        if (!screen->races[0].status)
            screen->races[0].status = 1;
        for (int index = 0; index < 33; ++index) {
            TreeRace &event = screen->races[index];
            auto unlock = [&](int child) {
                if (!screen->races[child].status)
                    screen->races[child].status = 1;
            };
            switch (event.status) {
            case 0:
                bind_tree_event_image(screen->tree, index, event.type, 1);
                break;
            case 1:
            case 2:
                bind_tree_event_image(screen->tree, index, event.type, 0);
                break;
            case 3:
                bind_tree_medal_image(screen->tree, index, 3);
                if (event.bronze_branch != -1)
                    unlock(event.bronze_branch);
                break;
            case 4:
            case 5:
                bind_tree_medal_image(screen->tree, index, event.status);
                unlock(int(event.branches[0]));
                unlock(int(event.branches[1]));
                break;
            }
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace nfs::application::frontend
