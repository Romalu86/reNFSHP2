#include "gui_images.hpp"
#include <cstring>
namespace nfs::gui::engine::images {

int find_separator(const String *text, const char *separator) {
    if (separator && !text->wide)
        if (const auto *data = static_cast<const char *>(string_data(text)))
            if (const char *match = std::strstr(data, separator))
                return int(match - data);
    return -1;
}
// Each removal allocates the shortened string, including its terminator.
void erase_character(String *text, int index) {
    if (index < 0 || index >= text->length || !text->length)
        return;
    const Dword width = text->wide ? 2 : 1;
    auto *copy = static_cast<Byte *>(kernel::memory_allocate(nullptr, text->length * width, pool));
    if (!copy)
        return;
    copy[0] = 0;
    if (width == 2)
        copy[1] = 0;
    if (const auto *data = static_cast<const Byte *>(string_data(text))) {
        std::memcpy(copy, data, index * width);
        std::memcpy(copy + index * width, data + (index + 1) * width,
                    (text->length - index) * width);
    }
    if (text->data)
        kernel::memory_free(text->data);
    text->data = copy;
    text->reference = 0;
    --text->length;
    text->wide = width == 2;
}

void trim_left(String *text, int code) {
    if (code)
        while (text->length && pending::character_at(text, 0) == Dword(int(char(code))))
            erase_character(text, 0);
}

void trim_right(String *text, int code) {
    if (code)
        while (text->length && pending::character_at(text, text->length - 1) == Dword(int(char(code))))
            erase_character(text, text->length - 1);
}
} // namespace nfs::gui::engine::images
