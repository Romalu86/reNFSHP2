#include "../widgets/gui_widget_edit.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
#include <cwchar>
namespace nfs::gui::engine::images {
void erase_character(String *, int);
}
namespace nfs::gui {
void erase_string_character(String *text, int index) {
    engine::images::erase_character(text, index);
}

void insert_string_character(String *text, int position, char character) {
    if (!character)
        return;
    if (text->wide) {
        insert_string_wide_character(text, position,
                                     wchar_t(static_cast<unsigned char>(character)));
        return;
    }
    if (position < 0 || position > text->length)
        return;
    auto *copy = static_cast<char *>(allocate(text->length + 2));
    if (!copy)
        return;
    copy[0] = copy[1] = 0;
    const auto *data = static_cast<const char *>(string_data(text));
    if (data)
        std::strcpy(copy, data);
    copy[position] = character;
    if (data)
        for (int index = position + 1; index <= text->length + 1; ++index)
            copy[index] = data[index - 1];
    if (text->data)
        release(text->data);
    text->data = copy;
    text->reference = text->wide = 0;
    ++text->length;
}

void insert_string_wide_character(String *text, int position, wchar_t character) {
    if (!character)
        return;
    if (!text->wide && string_data(text) && !string_make_wide(text))
        return;
    if (position < 0 || position > text->length)
        return;
    if (!text->wide && string_data(text))
        return;
    auto *copy = static_cast<wchar_t *>(allocate(text->length * 2 + 4));
    if (!copy)
        return;
    copy[0] = copy[1] = 0;
    const auto *data = static_cast<const wchar_t *>(string_data(text));
    if (data)
        std::wcscpy(copy, data);
    copy[position] = character;
    if (data)
        for (int index = position + 1; index <= text->length + 1; ++index)
            copy[index] = data[index - 1];
    if (text->data)
        release(text->data);
    text->reference = 0;
    text->data = copy;
    ++text->length;
    text->wide = 1;
}
} // namespace nfs::gui
