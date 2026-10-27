#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../lifecycle/gui_font_metrics.hpp"
#include <cctype>
#include <cwctype>
namespace nfs::gui::engine {
// Legacy narrow/wide CRT locale mapping remains an ABI boundary.
void uppercase(String *string) {
    const Dword wide = string->wide;
    const int length = string->length;
    const void *source = gui::string_data(string);
    if (!source || !length)
        return;
    if (!wide) {
        auto *target = static_cast<char *>(string->data);
        if (!target) {
            target = static_cast<char *>(
                kernel::memory_allocate(nullptr, length + 1, gui::pool));
            if (!target)
                return;
            if (length > 0) {
                Dword remaining = Dword(string->length);
                auto *input = static_cast<const signed char *>(source);
                auto *output = target;
                do {
                    *output++ = char(std::toupper(*input++));
                } while (--remaining);
            }
            target[string->length] = 0;
            string->data = target;
            string->reference = string->wide = 0;
        } else
            for (; *target; ++target)
                *target = char(std::toupper(static_cast<signed char>(*target)));
    } else {
        auto *target = static_cast<std::uint16_t *>(string->data);
        if (!target) {
            target = static_cast<std::uint16_t *>(
                kernel::memory_allocate(nullptr, length * 2 + 2, gui::pool));
            if (!target)
                return;
            if (length > 0) {
                Dword remaining = Dword(string->length);
                auto *input = static_cast<const std::uint16_t *>(source);
                auto *output = target;
                do {
                    *output++ = std::uint16_t(std::towupper(*input++));
                } while (--remaining);
            }
            target[string->length] = 0;
            string->data = target;
            string->reference = 0;
            string->wide = 1;
        } else
            for (; *target; ++target)
                *target = std::uint16_t(std::towupper(*target));
    }
}

fonts::faces::Face *get_font_face(int index) {
    auto *manager = reinterpret_cast<fonts::faces::Manager *>(game::runtime::system(20));
    if (index < 0 || index >= manager->count)
        return nullptr;
    return manager->faces + index;
}

fonts::Font *get_font(int index) {
    auto *face = get_font_face(index);
    if (!face)
        return nullptr;
    if (!face->font)
        fonts::faces::load(face);
    return face->font;
}
kernel::numeric::Extended retained_font_height(int index, float scale) {
    alignas(16) unsigned char value[10];
    font_ascent_native80(index, scale, value);
    return kernel::numeric::Extended::from_native80(value, kernel::numeric::current_format());
}

long double font_height(int index, float scale) {
    return double(retained_font_height(index, scale));
}
// Instructions include the successful match branch; the old analyzer incorrectly pruned it after
// its string-length inference.
int find_font(const String *name) {
    String wanted;
    gui::string_construct(&wanted);
    try {
        gui::string_copy(&wanted, name);
        uppercase(&wanted);
        for (int index = 0; index < game::runtime::at<int>(game::runtime::system(20), 8);
             ++index) {
            String filename, stem;
            gui::string_construct(&filename);
            gui::string_construct(&stem);
            try {
                gui::string_assign(&filename, reinterpret_cast<const char *>(get_font_face(index)));
                if (filename.length) {
                    const char *text = gui::string_ascii(&filename);
                    int length = 0;
                    while (length < filename.length && text[length] != '.')
                        ++length;
                    if (length == filename.length)
                        length = -1;
                    gui::string_substring(&filename, 0, length, &stem);
                    uppercase(&stem);
                    if (gui::string_equal(&stem, &wanted)) {
                        gui::string_destruct(&stem);
                        gui::string_destruct(&filename);
                        gui::string_destruct(&wanted);
                        return index;
                    }
                }
            } catch (...) {
                gui::string_destruct(&stem);
                gui::string_destruct(&filename);
                throw;
            }
            gui::string_destruct(&stem);
            gui::string_destruct(&filename);
        }
    } catch (...) {
        gui::string_destruct(&wanted);
        throw;
    }
    gui::string_destruct(&wanted);
    return 0;
}
} // namespace nfs::gui::engine
