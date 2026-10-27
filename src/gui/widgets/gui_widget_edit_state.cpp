#include "gui_widget_text.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <ctype.h>
#include <wctype.h>
#include <cstring>
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::invoke;

void set_edit_character_limit(Widget *widget, int limit) {
    at<int>(widget, 308) = limit;
    if (!limit || at<int>(widget, 308) >= invoke<const String *>(widget, 8)->length)
        return;
    String shortened;
    string_construct(&shortened);
    string_copy(&shortened, invoke<const String *>(widget, 8));
    try {
        string_substring(&shortened, 0, at<int>(widget, 308), &shortened);
        invoke(widget, 7, &shortened);
    } catch (...) {
        string_destruct(&shortened);
        throw;
    }
    string_destruct(&shortened);
}

Dword __fastcall edit_text_drawing_state(Widget *widget, void *, Dword render_address) {
    auto *render = reinterpret_cast<engine::Engine *>(render_address);
    if (!(at<Dword>(widget, 324) & 16) && !(widget->flags & 1) &&
        (invoke<Dword>(widget, 9) || application::pending::captured_widget_id == widget->id)) {
        invoke(render, 50, Dword(4));
        return 1;
    }
    return apply_drawing_state(widget, nullptr, render);
}

void __fastcall edit_text_accept(Widget *widget, void *) {
    if (!(at<Dword>(widget, 324) & 4))
        events::dispatch(widget, 64, 0, 1);
}
// The caller supplies one ignored argument (RET4).
void __fastcall validate_edit_text(Widget *widget, void *, Dword) {
    const String *text = invoke<const String *>(widget, 8);
    const Dword result =
        events::dispatch(widget, 63, reinterpret_cast<Dword>(string_ascii(text)), 0);
    if (result)
        events::post_widget(widget, 60, 0, result);
}

Dword __fastcall edit_text_event(Widget *widget, void *, Dword event, Dword first, Dword second) {
    switch (event) {
    case 58:
        set_edit_character_limit(widget, int(first));
        return 1;
    case 59:
        invoke(widget, 49);
        break;
    case 60:
        invoke(widget, 50, second);
        break;
    case 61:
        invoke(widget, 51, first);
        break;
    case 62:
        return invoke<Dword>(widget, 48, first, second);
    case 63:
        return invoke<Dword>(widget, 53, first);
    case 64:
        invoke(widget, 52, second);
        break;
    }
    return pending::general_event(reinterpret_cast<GeneralWidget *>(widget), nullptr, event, first,
                                  second);
}
// The final three words are a Point passed by value.
void __fastcall draw_edit_caret(Widget *, void *, engine::Engine *render, Dword point_methods,
                                Dword x, Dword y) {
    if (invoke<Dword>(render, 59))
        return;
    const Dword height = invoke<Dword>(render, 17);
    if (!height)
        return;
    const auto *viewport = engine::current_viewport_bounds(&render->base);
    Point start{reinterpret_cast<const Destroy *>(point_methods), int(x + Dword(viewport->left)),
                int(y + Dword(viewport->top))};
    Point end{engine::point_methods, start.x, int(Dword(start.y) + height)};
    Dword colour;
    invoke(render, 23, &colour, Dword(9));
    invoke(render, 46, &start, &end, colour, colour);
    engine::destroy_point(&end);
}
// Character classification uses the current CRT locale.
Dword __fastcall reject_edit_character(Widget *widget, void *, Dword character, Dword position) {
    const int narrow = static_cast<signed char>(character);
    auto classified = [&](int mask) {
        return _isctype(narrow, mask) || iswctype(wint_t(character), wctype_t(mask));
    };
    if (at<Dword>(widget, 320)) {
        if (at<Dword>(widget, 320) & 65536) {
            auto *render = active_engine();
            if (!render)
                return 1;
            const Dword font = invoke<Dword>(render, 14, inherited_font_style(widget));
            Dword metrics[6];
            const Dword available = invoke<Dword>(render, 16, character, metrics);
            invoke(render, 14, font);
            if (!available)
                return 1;
        }
        if ((at<Dword>(widget, 320) & 4) && classified(259))
            return 1;
        if ((at<Dword>(widget, 320) & 8) && classified(4))
            return 1;
        if ((at<Dword>(widget, 320) & 16) && classified(16))
            return 1;
        const Dword filter = at<Dword>(widget, 320);
        if ((filter & 32) && character == 32)
            return 1;
        if ((filter & 64) && character == 32 && !position)
            return 1;
        if ((filter & 128) && (character == '&' || character == '<' || character == '>'))
            return 1;
        if ((filter & 4096) &&
            ((!position && character == 32) || std::strchr("\":?<>/*", int(character))))
            return 1;
        if ((at<Dword>(widget, 320) & 8192) &&
            ((!position && character == 32) || std::strchr("\":?<>/*", int(character)) ||
             character == '\\'))
            return 1;
        if ((at<Dword>(widget, 320) & 16384) &&
            ((!position && character == 32) || std::strchr("\":?<>/*", int(character)) ||
             character == '.' || character == '\\'))
            return 1;
        if (at<Dword>(widget, 320) & 32768)
            return classified(4) || character == '-' ? 0 : 1;
    }
    if (!classified(259) && !classified(4) && character != 32 && !classified(16) &&
        character < 33) {
        if (character != 10 || !(at<Dword>(widget, 324) & 128) || !(at<Dword>(widget, 264) & 1))
            return 1;
    }
    return 0;
}
} // namespace nfs::gui::widgets::family
