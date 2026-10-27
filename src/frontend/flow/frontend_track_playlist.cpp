#include "frontend_scene_widgets.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "nfs/gui/resources/gui_format.hpp"
namespace nfs::profiles {
gui::widgets::family::ListItem *construct_profile_list_item(gui::widgets::family::ListItem *);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using family::ListBox;
using family::ListItem;
using game::runtime::at;
using game::runtime::invoke;
struct ScopedTrackItem {
    ListItem value;
    ScopedTrackItem() {
        profiles::construct_profile_list_item(&value);
    }
    ~ScopedTrackItem() {
        gui::string_destruct(&value.tooltip);
        gui::string_destruct(&value.text);
        value.methods = reinterpret_cast<const Method *>(gui::base_methods);
    }
};
static void *track_progress() {
    return pending::active_event_progress(game::runtime::system(72));
}

void clear_track_playlist(ListBox *list, int count) {
    invoke<void>(&list->items, 1);
    for (int index = 0; index < count; ++index) {
        const Dword row = invoke<Dword>(&list->items, 3);
        ScopedTrackItem item;
        pending::LocalFrontendString number;
        gui::string_format_ascii(&number.value, "%d", index + 1);
        gui::string_copy(&item.value.text, &number.value);
        invoke<void>(&list->items, 7, &item.value, row, Dword(0));
    }
}

bool track_playlist_empty(ListBox *list) {
    auto *item = invoke<ListItem *>(&list->items, 9, Dword(0), Dword(1));
    return !item || !item->text.length;
}

void unpack_track_choice(Dword choice, Dword *track, Dword *direction) {
    *track = choice & 65535;
    *direction = (choice >> 16) & 65535;
}

void read_track_playlist_row(ListBox *list, Dword row, Dword *track, Dword *direction, Dword *laps) {
    const Dword choice = invoke<ListItem *>(&list->items, 9, row, Dword(1))->data;
    *track = choice & 65535;
    *direction = (choice >> 16) & 65535;
    *laps = invoke<ListItem *>(&list->items, 9, row, Dword(3))->data;
}

bool track_playlist_full(ListBox *list) {
    auto *item =
        invoke<ListItem *>(&list->items, 9, (at<Dword>(track_progress(), 0) >> 1 & 7) + 1, Dword(1));
    return item && item->text.length;
}

Dword next_track_playlist_row(ListBox *list) {
    auto *item =
        invoke<ListItem *>(&list->items, 9, (at<Dword>(track_progress(), 0) >> 1 & 7) + 1, Dword(1));
    if (!item || !item->text.length) {
        for (Dword row = 0; row < (at<Dword>(track_progress(), 0) >> 1 & 7) + 2; ++row) {
            item = invoke<ListItem *>(&list->items, 9, row, Dword(1));
            if (!item || !item->text.length)
                return row;
        }
    }
    return Dword(-1);
}

Dword append_track_playlist(ListBox *list, const gui::String *name, Dword laps, Dword track,
                           int direction) {
    if (track_playlist_full(list))
        return Dword(-1);
    Dword row = 0;
    while (row < (at<Dword>(track_progress(), 0) >> 1 & 7) + 2) {
        auto *current = invoke<ListItem *>(&list->items, 9, row, Dword(1));
        if (!current || !current->text.length)
            break;
        ++row;
        if (row >= (at<Dword>(track_progress(), 0) >> 1 & 7) + 2)
            return Dword(-1);
    }
    ScopedTrackItem item;
    gui::string_copy(&item.value.text, name);
    item.value.data = (track & 65535) ^ (Dword(direction) << 16);
    invoke<void>(&list->items, 7, &item.value, row, Dword(1));
    pending::LocalFrontendString direction_text;
    gui::string_reference(&direction_text.value, direction == 1   ? 693
                                                 : direction == 2 ? 694
                                                 : direction == 3 ? 695
                                                                  : 692);
    gui::string_copy(&item.value.text, &direction_text.value);
    item.value.data = Dword(direction);
    invoke<void>(&list->items, 7, &item.value, row, Dword(2));
    gui::string_format_ascii(&direction_text.value, "%d", laps);
    gui::string_copy(&item.value.text, &direction_text.value);
    item.value.data = laps;
    invoke<void>(&list->items, 7, &item.value, row, Dword(3));
    return row;
}

void remove_last_track_playlist(ListBox *list) {
    Dword row = next_track_playlist_row(list);
    if (!row)
        return;
    if (row == Dword(-1))
        row = (at<Dword>(track_progress(), 0) >> 1 & 7) + 2;
    ScopedTrackItem item;

    static char empty_text[4]{};
    gui::string_assign(&item.value.text, empty_text);
    --row;
    for (Dword column = 1; column < 4; ++column)
        invoke<void>(&list->items, 7, &item.value, row, column);
}
} // namespace nfs::application::frontend
