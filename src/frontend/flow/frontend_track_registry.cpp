#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_track_selection.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
void *create_track_selection(void *);
void *create_image_wheel(void *);
Dword __cdecl track_next_pressed(void *, Dword, Dword, Dword);
Dword __cdecl track_previous_pressed(void *, Dword, Dword, Dword);
Dword __cdecl toggle_track_description(void *, Dword, Dword, Dword);
static ClassRecord track_classes[2];
static CallbackRecord track_callbacks[3];

static void unregister_track_classes() {
    for (unsigned i = 2; i; --i)
        unregister_class(track_classes + i - 1);
}

static void register_track_classes() {
    register_class(track_classes, "TrackSelectScreen", create_track_selection, "GInterface");
    register_class(track_classes + 1, "ImgWheel", create_image_wheel, "GWidget");
    std::atexit(unregister_track_classes);
}

static void unregister_track_callbacks() {
    for (unsigned i = 3; i; --i)
        unregister_callback(track_callbacks + i - 1);
}

static void register_track_callbacks() {
    register_callback(track_callbacks, family::button_class_key, "BEVENT_PRESSED", 43,
                      "TrackSelectScreen::OnNextPress", track_next_pressed, 0);
    register_callback(track_callbacks + 1, family::button_class_key, "BEVENT_PRESSED", 43,
                      "TrackSelectScreen::OnPrevPress", track_previous_pressed, 0);
    register_callback(track_callbacks + 2, family::button_class_key, "BEVENT_PRESSED", 43,
                      "TrackSelectScreen::ToggleDescription", toggle_track_description, 0);
    std::atexit(unregister_track_callbacks);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_track_classes, ".CRT$XCU_NFS_00690158",
                       register_track_classes);
NFS_KERNEL_INITIALIZER(startup_register_track_callbacks, ".CRT$XCU_NFS_0069015C",
                       register_track_callbacks);
} // namespace
} // namespace nfs::application::frontend
