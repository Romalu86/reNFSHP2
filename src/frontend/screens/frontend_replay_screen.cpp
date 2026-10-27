#include "frontend_auxiliary_screens.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;

void __fastcall initialize_replay_screen(ReplayScreen *screen, void *) {
    gui::widgets::pending::request_interface_layout(&screen->base, nullptr);
    screen->view = network_page_child(screen, "<VIEW>", family::button_class_key);
    screen->remove = network_page_child(screen, "<DELETE>", family::button_class_key);
    screen->replays = network_page_child(screen, "<REPLAYS>", pick_list_class_key);
    if (screen->replays) {
        // Default replay file names.
        {
            pending::LocalFrontendString text("Test replay 1");
            family::append_combo_item(screen->replays, &text.value, 0, 0);
        }
        {
            pending::LocalFrontendString text("Test replay 2");
            family::append_combo_item(screen->replays, &text.value, 1, 0);
        }
        {
            pending::LocalFrontendString text("Test replay 3");
            family::append_combo_item(screen->replays, &text.value, 2, 0);
        }
    }
}

void __fastcall replay_screen_child(ReplayScreen *screen, void *, Widget *sender, Dword event, Dword,
                                    Dword) {
    if (event != 43)
        return;
    Widget *button = sender && invoke<Dword>(sender, 2, family::button_class_key) ? sender : nullptr;
    if (button == screen->view) {
        family::finish_modal(&screen->base, 2, 0);
        pending::clear_screen_queue(system(96));
        return;
    }
    if (button != screen->remove)
        return;
    pending::MessageBox prompt;
    {
        pending::LocalFrontendString title("Delete"),
            text("Are you sure you want to delete the replay?");
        pending::construct_message_box(&prompt, &text.value, &title.value, 66);
    }
    try {
        if (pending::run_message_box(&prompt, nullptr, 0) == 2) {
            pending::MessageBox confirmation;
            {
                pending::LocalFrontendString text("Replay deleted.");
                pending::construct_message_box(&confirmation, &text.value);
            }
            try {
                pending::run_message_box(&confirmation, nullptr, 0);
            } catch (...) {
                pending::destroy_message_box(&confirmation);
                throw;
            }
            pending::destroy_message_box(&confirmation);
        }
    } catch (...) {
        pending::destroy_message_box(&prompt);
        throw;
    }
    pending::destroy_message_box(&prompt);
}
} // namespace nfs::application::frontend
