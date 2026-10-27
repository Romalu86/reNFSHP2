#include "../screens/frontend_menu_roots.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

void populate_tree_event_dialog(TreeEventDialog *dialog) {
    auto *event = dialog->event;
    if (!event)
        return;
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    if (auto *id = track_child(dialog, "<EVENT_ID>", family::text_class_key)) {
        gui::string_copy(&text.value, &lifecycle::construction::unresolved::shared_string);
        Dword reference;
        switch (event->type) {
        case 1:
            reference = 22;
            break;
        case 2:
            reference = 280;
            break;
        case 3:
            reference = 485;
            break;
        case 4:
        case 10:
            reference = 701;
            break;
        case 5:
            reference = 20;
            break;
        case 6:
            reference = 785;
            break;
        case 7:
            reference = 255;
            break;
        case 8:
            reference = 689;
            break;
        case 9:
            reference = 1215;
            break;
        case 11:
            reference = 1216;
            break;
        case 12:
            reference = 704;
            break;
        case 13:
            reference = 705;
            break;
        default:
            reference = 19;
        }
        gui::string_reference(&text.value, reference);
        invoke<void>(id, 7, &text.value);
    }
    if (auto *status = track_child(dialog, "<EVENT_STATUS>", family::text_class_key)) {
        Dword reference;
        if (event->type == 2 && pending::event_active(pending::active_event_progress(system(72))))
            reference = 1437;
        else
            switch (event->status) {
            case 1:
            case 2:
                reference = 292;
                break;
            case 3:
                reference = 699;
                break;
            case 4:
                reference = 698;
                break;
            case 5:
                reference = 697;
                break;
            default:
                reference = 291;
            }
        gui::string_reference(&text.value, reference);
        invoke<void>(status, 7, &text.value);
    }
    if (auto *objective = track_child(dialog, "<EVENT_OBJECTIVE>", family::text_class_key)) {
        if (event->type == 3 || event->type == 7) {
            const unsigned first = event->type == 3 ? 3 : 6;
            const int a = event->criteria[first], b = event->criteria[first + 1],
                      c = event->criteria[first + 2];
            pending::LocalFrontendString format;
            gui::string_reference(&format.value, event->objective);
            gui::string_format(&text.value, &format.value, a / 60, a % 60, b / 60, b % 60, c / 60,
                               c % 60);
        } else
            gui::string_reference(&text.value, event->objective);
        invoke<void>(objective, 7, &text.value);
        if (at<Dword>(system(72), 2312) == 6)
            if (auto *engine = gui::active_engine()) {
                pending::LocalFrontendString style("Xbodycopy");
                if (Dword found = invoke<Dword>(engine, 7, &style.value))
                    objective->font_style = found;
                invoke<void>(engine, 14, inherited_font_style(objective));
                at<int>(objective, 276) = invoke<int>(engine, 17) / 2;
            }
    }
    if (auto *reward = track_child(dialog, "<REWARD>", family::text_class_key)) {
        gui::string_reference(&text.value, 699);
        gui::string_append_reference(&text.value, 722);
        gui::string_append_reference(&text.value, 733);
        gui::string_append_reference(&text.value, 733);
        pending::LocalFrontendString amount;
        gui::string_format_ascii(&amount.value, "%d", event->rewards[0]);
        gui::string_append(&text.value, &amount.value);
        gui::string_append_reference(&text.value, 465);
        gui::string_append_ascii(&text.value, "\n");
        gui::string_append_reference(&text.value, 698);
        gui::string_append_reference(&text.value, 722);
        gui::string_append_reference(&text.value, 733);
        gui::string_append_reference(&text.value, 733);
        gui::string_format_ascii(&amount.value, "%d", event->rewards[1]);
        gui::string_append(&text.value, &amount.value);
        gui::string_append_reference(&text.value, 465);
        gui::string_append_ascii(&text.value, "\n");
        gui::string_append_reference(&text.value, 697);
        gui::string_append_reference(&text.value, 722);
        gui::string_append_reference(&text.value, 733);
        gui::string_append_reference(&text.value, 733);
        gui::string_format_ascii(&amount.value, "%d", event->rewards[2]);
        gui::string_append(&text.value, &amount.value);
        gui::string_append_reference(&text.value, 465);
        invoke<void>(reward, 7, &text.value);
    }
}
} // namespace nfs::application::frontend
