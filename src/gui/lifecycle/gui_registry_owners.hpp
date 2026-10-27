#pragma once
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::gui {
extern Container combo_boxes;
extern Container tree_nodes;
extern Container script_instances;
extern Container global_script_variables;
void initialize_global_script_variables();
void initialize_script_instances();
} // namespace nfs::gui
