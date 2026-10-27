#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_menu_controls.hpp"
#include "nfs/core/lifecycle/array_construction.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
void *create_rollover(void *);
void *create_type_spin(void *);
void *create_options_view(void *);
void *create_challenge_view(void *);
void *create_menu_spin(void *);
void *create_typewriter(void *);
void *create_text_wheel(void *);
static ClassRecord menu_control_classes[8];

static void unregister_menu_controls() {
    lifecycle::construction::objects::destroy_array(
        menu_control_classes, sizeof(ClassRecord), 8, unregister_class);
}

static void register_menu_controls() {
    register_class(menu_control_classes + 0, "RolloverButton", create_rollover, "GButton");
    register_class(menu_control_classes + 1, "TypeSpinCtl", create_type_spin, "GWidget");
    register_class(menu_control_classes + 2, "HP2OptionsView", create_options_view, "GWidget");
    register_class(menu_control_classes + 3, "HP2SingleChallengeView", create_challenge_view,
                   "GWidget");
    register_class(menu_control_classes + 4, "HP2SpinButton", create_menu_spin, "GButton");
    register_class(menu_control_classes + 5, "HP2TypeWriter", create_typewriter, "GWidget");
    register_class(menu_control_classes + 6, "HP2TypeWriterText", create_text_wheel, "GText");
    register_class(menu_control_classes + 7, "HP2EditText", create_profile_edit, "GEditText");
    std::atexit(unregister_menu_controls);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_menu_controls, ".CRT$XCU_NFS_0069016C",
                       register_menu_controls);
} // namespace
} // namespace nfs::application::frontend
