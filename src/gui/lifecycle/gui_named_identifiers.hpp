#pragma once
#include "../widgets/gui_widget_hooks.hpp"

namespace nfs::gui::widgets::identifiers {
extern NameRecord modify_control;
extern NameRecord replay_tests[3];
void initialize_modify_control();
void initialize_replay_tests();
} // namespace nfs::gui::widgets::identifiers
