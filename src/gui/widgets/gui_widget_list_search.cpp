#include "gui_widget_list.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/core/timing/timer.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cctype>
#include <cwctype>
#include <cwchar>
namespace nfs::gui {

void string_append_wide(String *text, const wchar_t *tail) {
    if (!tail)
        return;
    if (!text->wide && string_data(text) && !string_make_wide(text))
        return;
    const int added = int(std::wcslen(tail));
    if (!added)
        return;
    const int previous = text->length;
    auto *memory = static_cast<wchar_t *>(allocate((previous + added) * 2 + 2));
    if (!memory)
        return;
    *memory = 0;
    if (const void *source = string_data(text); source && previous)
        kernel::memory_copy(memory, source, previous * 2 + 2);
    std::wcscat(memory, tail);
    if (text->data)
        release(text->data);
    text->reference = 0;
    text->data = memory;
    text->length = previous + added;
    text->wide = 1;
}
// including mixed-width .
bool string_equal_prefix_folded(const String *first, const String *second, int length) {
    if (length < 0)
        return false;
    const void *left = string_data(first), *right = string_data(second);
    if (!left)
        return !right;
    if (!right)
        return false;
    const auto folded = [](const void *data, Dword wide, int index) {
        if (wide)
            return int(std::towlower(static_cast<const std::uint16_t *>(data)[index]));
        return int(
            static_cast<signed char>(std::tolower(static_cast<const signed char *>(data)[index])));
    };
    int index = 0;
    for (;;) {
        const int a = folded(left, first->wide, index), b = folded(right, second->wide, index);
        if (a != b)
            return length == 0;
        if (!a || !length)
            return true;
        ++index;
        --length;
    }
}
} // namespace nfs::gui
namespace nfs::gui::widgets::family {
using game::runtime::invoke;

int find_list_text(Widget *widget, const String *text, Dword flags) {
    if (!text->length)
        return -1;
    auto *list = list_box(widget);
    int start = flags & 2 ? list->cursor : 0;
    if (start < 1)
        start = 0;
    const auto matches = [&](int row) {
        auto *item = invoke<ListItem *>(&list->items, 9, row, Dword(0));
        if (!item || !item->text.length || ((flags & 4) && (item->flags & 2)))
            return false;
        return flags & 1 ? string_equal_prefix_folded(&item->text, text, text->length)
                         : string_equal(&item->text, text);
    };
    for (int row = start; row < int(list->items.grid.rows.count); ++row)
        if (matches(row))
            return row;
    for (int row = 0; row < start; ++row)
        if (matches(row))
            return row;
    return -1;
}

void __fastcall list_box_character(Widget *widget, void *, Dword character, Dword modifiers) {
    auto *list = list_box(widget);
    if (list->style & 128) {
        const Dword now = kernel::timer_count();
        if (int(kernel::timer_frequency) < int(now - list->search_time))
            string_copy(&list->search,
                        &lifecycle::construction::unresolved::shared_string);
        list->search_time = now;
        if (character) {
            if (!(modifiers & 4194304))
                string_append_character(&list->search, char(character));
            else {
                const wchar_t pair[] = {wchar_t(character), 0};
                if (pair[0])
                    string_append_wide(&list->search, pair);
            }
        }
        if (list->search.length) {
            const int row = find_list_text(widget, &list->search, 7);
            if (row >= 0) {
                if (invoke<Dword>(&list->items, 10, row, Dword(1)))
                    events::post_widget(widget, 68, 0, 0);
                if (row < int(list->items.grid.rows.count)) {
                    list->cursor = row;
                    ensure_list_box_row_visible(widget, row);
                    if (row < int(list->items.grid.rows.count))
                        list->anchor = row;
                }
            }
        }
    }
    style_result_35(widget, nullptr, character, modifiers);
}
} // namespace nfs::gui::widgets::family
