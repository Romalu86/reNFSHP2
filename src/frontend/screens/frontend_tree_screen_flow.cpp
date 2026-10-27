#include "frontend_menu_roots.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
namespace nfs::application::pending {
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
void __cdecl wait_tree_exit(void *);

void __fastcall resume_tree_screen(TreeScreen *screen, void *) {
    const bool pursuit = at<Dword>(system(72), 148) == 2;
    pending::LocalFrontendString catalogue(pursuit ? "Hot Pursuit" : "Championship");
    load_tree_events(screen, catalogue.release());
    screen->selection = at<int>(screen->preferences, pursuit ? 7540 : 7536);
    game::audio::play_frontend_event(pursuit ? 27 : 26, 90);
    if (auto *button = tree_race_button(screen, screen->selection))
        activate(button);
    enable_tree_selection(screen, 1);
    const int selection = screen->selection;
    at<int>(system(72), 102920) = selection;
    screen->tree->selection = selection;
    screen->tree->rotation = 0;
    if (screen->description) {
        TreeRace &event = screen->races[selection];
        screen->mode = int(event.type);
        invoke<void>(screen->description, 7,
                     &lifecycle::construction::unresolved::shared_string);
        pending::LocalFrontendString text;
        gui::string_reference(&text.value,
                              event.title ? event.title : screen->race_names[event.type]);
        invoke<void>(screen->description, 7, &text.value);
    }
    auto &notice = at<std::uint8_t>(screen->preferences, pursuit ? 7544 : 7545);
    if (!notice)
        return;
    notice = 0;
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, pursuit ? 1446 : 1447);
    pending::MessageBox message;
    pending::construct_message_box(&message, &text.value,
                                   &lifecycle::construction::unresolved::shared_string,
                                   4225);
    try {
        pending::run_message_box(&message, nullptr, 0);
        if (auto *button = tree_race_button(screen, 0))
            activate(button);
    } catch (...) {
        pending::destroy_message_box(&message);
        throw;
    }
    pending::destroy_message_box(&message);
}

void open_tree_event(TreeScreen *screen) {
    if (!screen->tree)
        return;
    zoom_tree_selection_in(screen->tree);
    pending::LocalFrontendString type("TreeEventDlg"), name("TreeEvent"),
        resource("system\\GDialogs.gui");
    auto *dialog = static_cast<TreeEventDialog *>(
        pending::create_widget(&resource.value, &name.value, &type.value, screen));
    if (dialog) {
        dialog->selection = Dword(screen->selection);
        dialog->event = screen->races + screen->selection;
        populate_tree_event_dialog(dialog);
        const int result = gui::run_modal(dialog, nullptr);
        pending::destroy_screen(dialog);
        zoom_tree_selection_out(screen->tree);
        if (result == 2)
            family::finish_modal(&screen->base, 4, 0);
    }
}
// The saved stack copy preserves the result before storing it in this screen.
Dword __fastcall finish_tree_screen(TreeScreen *screen, void *, Dword result, Dword source) {
    enable_tree_selection(screen, 0);
    auto *owner = reinterpret_cast<Widget *>(source);
    if (auto *tree =
            reinterpret_cast<TreeWidget *>(track_child(owner, "<TREE>", tree_widget_class_key))) {
        begin_tree_exit(tree);
        gui::run_modal(tree, wait_tree_exit);
    }
    screen->base.modal_result = result;
    return 1;
}
} // namespace nfs::application::frontend
