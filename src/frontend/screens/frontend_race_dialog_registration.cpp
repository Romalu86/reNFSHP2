#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../flow/frontend_race_results.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
static ClassRecord starting_grid_class, letterbox_class;
static ClassRecord pause_dialog_class, countdown_class;
static ClassRecord end_game_class;
static ClassRecord replay_controls_class;
static ClassRecord race_results_class, cop_results_class;

static void unregister_race_dialogs() {
    unregister_class(&pause_dialog_class);
    unregister_class(&starting_grid_class);
    unregister_class(&countdown_class);
    unregister_class(&race_results_class);
    unregister_class(&end_game_class);
    unregister_class(&replay_controls_class);
    unregister_class(&letterbox_class);
    unregister_class(&cop_results_class);
}

static void register_race_dialogs() {
    register_class(&pause_dialog_class, "HP2Pause", create_pause_dialog, "GDialog");
    register_class(&starting_grid_class, "HP2StartingGrid", create_starting_grid, "GDialog");
    register_class(&countdown_class, "HP2CountDown", create_countdown_widget, "GWidget");
    register_class(&race_results_class, "HP2RaceResults", create_race_results, "GDialog");
    register_class(&end_game_class, "HP2EndGame", create_end_game_dialog, "GDialog");
    register_class(&replay_controls_class, "HP2ReplayControls", create_replay_controls, "GDialog");
    register_class(&letterbox_class, "HP2LetterBox", create_letterbox, "GWidget");
    register_class(&cop_results_class, "HP2CopVsPerpResults", create_cop_results, "GDialog");
    std::atexit(unregister_race_dialogs);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_race_dialogs, ".CRT$XCU_NFS_00690114",
                       register_race_dialogs);
}
} // namespace nfs::application::frontend
