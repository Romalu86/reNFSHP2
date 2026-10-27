#include "gui_named_identifiers.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include <cstdlib>

namespace nfs::gui::widgets::identifiers {
NameRecord modify_control{};
NameRecord replay_tests[3]{};


void destroy_modify_control() {
    lifecycle::construction::objects::destroy_array(&modify_control, 12, 1, destroy_name_record);
}

 
void initialize_modify_control() {
    register_name(&modify_control, "MODIFYCONTROL");
    std::atexit(destroy_modify_control);
}


void destroy_replay_tests() {
    lifecycle::construction::objects::destroy_array(replay_tests, 12, 3, destroy_name_record);
}

 
void initialize_replay_tests() {
    register_name(replay_tests, "TEST1");
    register_name(replay_tests + 1, "TEST2");
    register_name(replay_tests + 2, "TEST3");
    std::atexit(destroy_replay_tests);
}
} // namespace nfs::gui::widgets::identifiers
