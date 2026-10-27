#include "frontend_menu_roots.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
namespace nfs::application::pending {
void configure_selected_event(void *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

TreeEventDialog *construct_tree_event_dialog(TreeEventDialog *dialog, void *parent) {
    gui::widgets::pending::construct_dialog(dialog, parent);
    gui::string_construct(&dialog->resource);
    dialog->event = nullptr;
    dialog->selection = 0;
    dialog->base.methods = tree_event_dialog_methods.data();
    return dialog;
}

void destroy_tree_event_dialog(TreeEventDialog *dialog) {
    dialog->base.methods = tree_event_dialog_methods.data();
    gui::string_destruct(&dialog->resource);
    family::dialog_destroy(&dialog->base);
}

void __fastcall initialize_tree_event_dialog(TreeEventDialog *dialog, void *) {
    family::dialog_initialize(&dialog->base, nullptr);
    if (auto *heading = track_child(dialog, "<EVENT_ID>", family::text_class_key))
        gui::string_copy(&dialog->resource, invoke<const gui::String *>(heading, 8));
}

Dword run_tree_event_dialog(TreeEventDialog *dialog, TreeRace *event, Dword selection) {
    dialog->event = event;
    dialog->selection = selection;
    populate_tree_event_dialog(dialog);
    return gui::run_modal(dialog, nullptr) == 2;
}
static void enqueue_event_page(const char *name, const char *type) {
    pending::LocalFrontendString page(name), kind(type);
    pending::enqueue_screen(system(96), page.release(), kind.release());
}

Dword __fastcall finish_tree_event_dialog(TreeEventDialog *dialog, void *, Dword result, Dword) {
    if (result == 2) {
        if (!dialog->event)
            return 0;
        if (!dialog->event->status && !launch::field<Dword>(8627868)) {
            pending::LocalFrontendString text;
            gui::string_reference(&text.value, 1724);
            game::audio::play_frontend_event(66, 63);
            pending::MessageBox message;
            pending::construct_message_box(
                &message, &text.value, &lifecycle::construction::unresolved::shared_string,
                257);
            try {
                pending::run_message_box(&message, nullptr, 0);
            } catch (...) {
                pending::destroy_message_box(&message);
                throw;
            }
            pending::destroy_message_box(&message);
            return 0;
        }
        pending::configure_selected_event(dialog);
        if (at<Dword>(system(72), 152) == 2) {
            void *progress = pending::active_event_progress(system(72));
            if (pending::event_active(progress) && !pending::event_complete_progress(progress)) {
                pending::LocalFrontendString text;
                gui::string_reference(&text.value, 1815);
                pending::MessageBox resume;
                pending::construct_message_box(&resume, &text.value, 3);
                try {
                    const Dword labels[] = {372, 1510, 460}, results[] = {2, 17, 3};
                    for (int i = 0; i < 3; ++i) {
                        pending::LocalFrontendString label;
                        gui::string_reference(&label.value, labels[i]);
                        pending::configure_message_button(&resume, i, &label.value, results[i]);
                    }
                    const int answer = pending::run_message_box(&resume, nullptr, 0);
                    if (answer != 2) {
                        if (answer != 17) {
                            pending::destroy_message_box(&resume);
                            return 0;
                        }
                        pending::LocalFrontendString question, title;
                        gui::string_reference(&question.value, 1329);
                        gui::string_reference(&title.value, 1330);
                        pending::MessageBox confirm;
                        pending::construct_message_box(&confirm, &title.value, &question.value,
                                                       66);
                        int confirmed;
                        try {
                            confirmed = pending::run_message_box(&confirm, nullptr, 0);
                        } catch (...) {
                            pending::destroy_message_box(&confirm);
                            throw;
                        }
                        pending::destroy_message_box(&confirm);
                        if (confirmed != 2) {
                            pending::destroy_message_box(&resume);
                            return 0;
                        }
                        pending::clear_event_progress(progress);
                    }
                } catch (...) {
                    pending::destroy_message_box(&resume);
                    throw;
                }
                pending::destroy_message_box(&resume);
            }
            enqueue_event_page("TourneyStatus", "TourneyStatus");
        } else
            enqueue_event_page("CarSelect", "CarSelectScreen");
    }
    dialog->base.modal_result = result;
    return 1;
}
} // namespace nfs::application::frontend
