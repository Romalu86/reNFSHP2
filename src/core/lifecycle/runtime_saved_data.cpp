#include "nfs/network/session/network_player_state.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::network::players::pending {
const VehicleName vehicle_names[] = {
    {"Crown Victoria", 7},
    {"Lotus Elise", 9},
    {"Vauxhall VX220", 21},
    {"Opel Speedster", 17},
    {"Ford TS50", 18},
    {"Mercedes CL55 AMG", 3},
    {"Jaguar XKR", 22},
    {"BMW M5", 14},
    {"BMW Z8", 23},
    {"HSV Coup\351 GTS", 13},
    {"SVT Mustang Cobra R", 5},
    {"Chevrolet Corvette Z06", 6},
    {"Dodge Viper GTS", 20},
    {"Ferrari 360 Spider", 15},
    {"Porsche 911 Turbo", 0},
    {"V12 Vanquish", 19},
    {"Ferrari 550 Barchetta", 1},
    {"Lamborghini Diablo", 8},
    {"Porsche Carrera GT", 2},
    {"Lamborghini Murci\351lago", 16},
    {"Mercedes CLK GTR", 4},
    {"Ferrari F50", 12},
    {"McLaren F1", 10},
    {"McLaren F1 LM", 11},
    {"Unknown Vehicle", 0},
};
}
namespace nfs::graphics::unresolved {
wchar_t error_message[] = {85, 110, 107, 110, 111, 119, 110, 32, 112, 114,
                                    111, 98, 108, 101, 109, 32, 105, 110, 105, 116,
                                    105, 97, 108, 105, 122, 105, 110, 103, 32, 103,
                                    114, 97, 112, 104, 105, 99, 115, 46, 0};
}
namespace nfs::lifecycle::table_entries {
extern const char replay_name[] = "replay.hp2";
}
namespace nfs::lifecycle::construction::unresolved {
gui::String shared_string{}; // 20-byte storage initialized before application entry.
}
