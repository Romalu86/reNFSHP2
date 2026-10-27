#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "hud_display.hpp"
#include "nfs/core/lifecycle/array_construction.hpp"
#include <cstdlib>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
static ClassRecord instrument_classes[3];
static ClassRecord needle_class;
static ClassRecord view_classes[4];
static ClassRecord progress_classes[2];

static void unregister_progress() {
    lifecycle::construction::objects::destroy_array(progress_classes, sizeof(ClassRecord), 2,
                                                   unregister_class);
}

static void register_progress() {
    register_class(progress_classes, "WCuffometer", create_cuffometer, "GWidget");
    register_class(progress_classes + 1, "ProgressBar", create_progress_bar, "GStaticImage");
    std::atexit(unregister_progress);
}

static void unregister_instruments() {
    lifecycle::construction::objects::destroy_array(instrument_classes, sizeof(ClassRecord), 3,
                                                   unregister_class);
}

static void register_instruments() {
    register_class(instrument_classes, "HudTachometer", create_tachometer, "MNeedle");
    register_class(instrument_classes + 1, "HudSpeedometer", create_speedometer, "MNeedle");
    register_class(instrument_classes + 2, "HudDashboard", create_dashboard, "GWidget");
    std::atexit(unregister_instruments);
}

static void unregister_needle() {
    lifecycle::construction::objects::destroy_array(&needle_class, sizeof(ClassRecord), 1,
                                                   unregister_class);
}

static void register_needle() {
    register_class(&needle_class, "MNeedle", create_needle, "GWidget");
    std::atexit(unregister_needle);
}

static void unregister_view() {
    lifecycle::construction::objects::destroy_array(view_classes, sizeof(ClassRecord), 4,
                                                   unregister_class);
}

static void register_view() {
    register_class(view_classes, "HudView", create_view, "GWidget");
    register_class(view_classes + 1, "HudText", create_text, "FlashText");
    register_class(view_classes + 2, "HudStaticArt", create_static_art, "GStaticImage");
    register_class(view_classes + 3, "HudRearView", create_rear_view, "GWidget");
    std::atexit(unregister_view);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_instruments, ".CRT$XCU_NFS_006900AC",
                       register_instruments);
NFS_KERNEL_INITIALIZER(startup_register_needle, ".CRT$XCU_NFS_006900E0",
                       register_needle);
NFS_KERNEL_INITIALIZER(startup_register_view, ".CRT$XCU_NFS_006900B8",
                       register_view);
NFS_KERNEL_INITIALIZER(startup_register_progress, ".CRT$XCU_NFS_00690074",
                       register_progress);
} // namespace
} // namespace nfs::application::frontend::hud
