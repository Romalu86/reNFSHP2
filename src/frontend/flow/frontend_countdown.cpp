#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_simulation.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::application::pending {
gui::String *construct_grouped_integer_text(gui::String *, int);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;
const char *__cdecl flash_text_class_key();
void play_countdown_audio(void *, int);
static int countdown_thresholds[5];

static void initialize_countdown_three() { countdown_thresholds[4] = 180; }

static void initialize_countdown_two() { countdown_thresholds[3] = 120; }

static void initialize_countdown_one() { countdown_thresholds[2] = 60; }

static void initialize_countdown_go() { countdown_thresholds[1] = 0; }

static void initialize_countdown_finish() { countdown_thresholds[0] = -60; }
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_countdown_three, ".CRT$XCU_NFS_00690100",
                       initialize_countdown_three);
NFS_KERNEL_INITIALIZER(startup_initialize_countdown_two, ".CRT$XCU_NFS_00690104",
                       initialize_countdown_two);
NFS_KERNEL_INITIALIZER(startup_initialize_countdown_one, ".CRT$XCU_NFS_00690108",
                       initialize_countdown_one);
NFS_KERNEL_INITIALIZER(startup_initialize_countdown_go, ".CRT$XCU_NFS_0069010C",
                       initialize_countdown_go);
NFS_KERNEL_INITIALIZER(startup_initialize_countdown_finish, ".CRT$XCU_NFS_00690110",
                       initialize_countdown_finish);
void clear_countdown_text(CountdownWidget *screen) {
    pending::LocalFrontendString empty("");
    invoke<void>(screen->text, 7, &empty.value);
}
void set_countdown_number(CountdownWidget *screen, int number) {
    gui::String text;
    pending::construct_grouped_integer_text(&text, number);
    try {
        invoke<void>(screen->text, 7, &text);
    } catch (...) {
        gui::string_destruct(&text);
        throw;
    }
    gui::string_destruct(&text);
}
} // namespace

void __fastcall initialize_countdown(CountdownWidget *screen, void *) {
    gui::widgets::pending::request_general_layout(&screen->base, nullptr);
    auto *text = static_cast<Widget *>(pending::find_child(screen, "<COUNT>"));
    screen->text = text && invoke<Dword>(text, 2, flash_text_class_key) ? text : nullptr;
    if (screen->text)
        clear_countdown_text(screen);
    screen->phase = 4;
}
// Tick argument is ignored.
void __fastcall tick_countdown(CountdownWidget *screen, void *, Dword) {
    if (screen->phase == -1 && game::runtime::race_started)
        return;
    if (!game::start_deadline) {
        if (screen->phase < 4) {
            screen->phase = 4;
            clear_countdown_text(screen);
        }
        return;
    }
    if (!screen->text || screen->phase < 0)
        return;
    const auto remaining = std::int32_t(game::start_deadline - graphics::unresolved::camera_state);
    if (remaining < countdown_thresholds[0] && screen->phase >= 0) {
        screen->phase = -1;
        clear_countdown_text(screen);
        play_countdown_audio(system(84), -1);
    }
    int phase;
    if (remaining < countdown_thresholds[1])
        phase = 0;
    else if (remaining < countdown_thresholds[2])
        phase = 1;
    else if (remaining < countdown_thresholds[3])
        phase = 2;
    else if (remaining < countdown_thresholds[4])
        phase = 3;
    else
        return;
    if (screen->phase < phase + 1)
        return;
    screen->phase = phase;
    if (!phase) {
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, 1367);
        invoke<void>(screen->text, 7, &text.value);
    } else {
        set_countdown_number(screen, phase);
    }
    invoke<void>(screen->text, 48);
    play_countdown_audio(system(84), phase);
}
} // namespace nfs::application::frontend
