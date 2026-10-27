#include "gui_ime.hpp"
#include <cstdlib>
#include <mbstring.h>
namespace nfs::gui::widgets::ime {

void open_context(Edit *edit) {
    HIMC current = ImmGetContext(input_window());
    if (current) {
        if (!ImmGetOpenStatus(current))
            ImmSetOpenStatus(current, TRUE);
        ImmNotifyIME(current, 21, 4, 0);
        edit->composition_caret = 0;
        edit->pending_character = 0;
        edit->composing = 0;
        ImmReleaseContext(input_window(), current);
    }
}

void close_context(Edit *edit) {
    HIMC current = ImmGetContext(input_window());
    if (current) {
        if (ImmGetOpenStatus(current))
            ImmSetOpenStatus(current, FALSE);
        edit->composition_caret = 0;
        edit->pending_character = 0;
        edit->composing = 0;
        ImmNotifyIME(current, 21, 4, 0);
        ImmReleaseContext(input_window(), current);
    }
}

bool context_open(Edit *) {
    bool open = false;
    HIMC current = ImmGetContext(input_window());
    if (current) {
        open = ImmGetOpenStatus(current) != FALSE;
        ImmReleaseContext(input_window(), current);
    }
    return open;
}

void detach_context(Edit *edit) {
    edit->detached_context = ImmAssociateContext(input_window(), nullptr);
    edit->composing = 0;
    edit->disabled = 1;
    edit->pending_character = 0;
}

void enable_context(Edit *edit) {
    ImmAssociateContext(input_window(), edit->detached_context);
    select_layout(edit, GetKeyboardLayout(0));
    edit->detached_context = nullptr;
    edit->disabled = 0;
    edit->composing = 0;
    edit->pending_character = 0;
}

std::uint8_t context_disabled(Edit *edit) {
    return edit->disabled;
}

std::uint16_t decode_character(Edit *, std::uint16_t character) {
    Dword bytes = character;
    if (character & 65280)
        bytes = ((character & 255) << 8) | (character >> 8);
    wchar_t decoded = 0;
    if (!MultiByteToWideChar(codepage, 0, reinterpret_cast<const char *>(&bytes), 2, &decoded, 2)) {
        GetLastError();
        decoded = 0;
    }
    return std::uint16_t(decoded);
}

Dword composition_string(Edit *, HIMC current, Dword kind, wchar_t *output, Dword bytes) {
    if (!unicode_composition) {
        char narrow[256];
        const LONG count = ImmGetCompositionStringA(current, kind, narrow, 255);
        if (count < 0) {
            *output = 0;
            return 0;
        }
        narrow[count] = 0;
        MultiByteToWideChar(codepage, 0, narrow, -1, output, int(bytes >> 1));
    } else {
        const LONG count = ImmGetCompositionStringW(current, kind, output, bytes);
        if (count < 0) {
            *output = 0;
            return 0;
        }
        *reinterpret_cast<std::uint16_t *>(reinterpret_cast<std::uint8_t *>(output) +
                                           (Dword(count) & ~Dword(1))) = 0;
    }
    return 1;
}

int compact_attributes(Edit *, const unsigned char *text, std::uint8_t *attributes) {
    auto *output = attributes;
    for (const auto *character = text; *character; character = _mbsinc(character))
        *output++ = attributes[character - text];
    return int(output - attributes);
}

std::uint16_t first_composition_character(Edit *) {
    wchar_t decoded[256]{};
    int result;
    if (!unicode_composition) {
        char narrow[256]{};
        if (!ImmGetCompositionStringA(context, GCS_COMPSTR, narrow, sizeof(narrow)))
            return 0;
        result = MultiByteToWideChar(codepage, 0, narrow, 256, decoded, 256);
    } else {
        result = ImmGetCompositionStringW(context, GCS_COMPSTR, decoded, sizeof(decoded));
    }
    return result ? std::uint16_t(decoded[0]) : 0;
}

void toggle_settings(Edit *) {
    if (input_window())
        ImmSimulateHotKey(input_window(), 82);
}

void select_layout(Edit *edit, HKL layout) {
    edit->language = std::uint16_t(reinterpret_cast<Dword>(layout));
    char text[8];
    GetLocaleInfoA(edit->language, LOCALE_IDEFAULTANSICODEPAGE, text, sizeof(text));
    codepage = Dword(std::atoi(text));
    edit->properties = ImmGetProperty(layout, IGP_PROPERTY);
    unicode_composition = edit->wide_os && (edit->properties & IME_PROP_UNICODE);
}

void notify_context(Edit *edit, Dword notification, Dword) {
    if (notification == 1 || notification == 2) {
        edit->composing = 0;
        edit->pending_character = 0;
    } else if (notification == 6) {
        conversion_status(edit);
    } else if (notification == 8) {
        HIMC current = ImmGetContext(input_window());
        if (current) {
            if (!ImmGetOpenStatus(current)) {
                edit->pending_character = 0;
                edit->composing = 0;
            }
            ImmReleaseContext(input_window(), current);
        }
    }
}
// The loops read flag columns only; no status text is produced.
void conversion_status(Edit *) {
    static const volatile Dword conversion_flags[]{32, 512, 2048, 8,   64,   2,
                                                  1,  256, 16,   128, 1024, 0};
    static const volatile Dword sentence_flags[]{4, 8, 1, 2, 16, 0};
    HIMC current = ImmGetContext(input_window());
    if (current) {
        DWORD conversion = 0, sentence = 0;
        if (ImmGetConversionStatus(current, &conversion, &sentence)) {
            for (unsigned index = 0; conversion_flags[index]; ++index) {
            }
            for (unsigned index = 0; sentence_flags[index]; ++index) {
            }
        }
        ImmReleaseContext(input_window(), current);
    }
}
} // namespace nfs::gui::widgets::ime
