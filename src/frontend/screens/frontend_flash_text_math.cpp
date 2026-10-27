#include "../flow/frontend_flash_text.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../../gui/lifecycle/gui_render_numeric_x87.hpp"

#pragma STDC FENV_ACCESS ON

namespace nfs::application::frontend {
using namespace gui::widgets;
void __fastcall flash_text_timer(FlashText *widget, void *, Dword timer) {
    if (widget->active && timer == widget->timer) {
        const auto progress = gui::retained::number(widget->speed) * 0.02f + widget->progress;
        gui::retained::write_field(widget->progress, float(progress));
        if (progress >= 1.0L) {
            gui::retained::write_field(widget->progress, 0.0f);
            gui::retained::write_field(widget->active, std::uint8_t(0));
        }
    } else if (widget->style & 2)
        pending::set_widget_visible(widget, 0);
}

void __fastcall draw_flash_text(FlashText *widget, void *, gui::engine::Engine *render) {
    using gui::retained::number;
    if (widget->progress != 0) {
        const Dword initial_colour = widget->base.base.colour;
        const auto scale_x = number(widget->grow_x) * widget->progress + 1.0f;
        const float stored_x = float(scale_x);
        const auto scale_y = number(widget->grow_y) * widget->progress + 1.0f;
        const float stored_y = float(scale_y);
        const auto blend = [&](long double amount) {
            const auto inverse = 1.0L - amount;
            Dword colour = initial_colour;
            for (int shift : {24, 16, 8, 0}) {
                const float source = float((initial_colour >> shift) & 255);
                const auto destination = gui::retained::integer(
                    (widget->destination_colour >> shift) & 255);
                const auto destination_product = destination * amount;
                const auto source_product = number(source) * inverse;
                const Dword channel = gui::retained::truncate(
                    destination_product + source_product) & 255;
                colour = (colour & ~(Dword(255) << shift)) | (channel << shift);
            }
            return colour;
        };
        const Dword first_colour = blend(number(widget->progress));
        render->scale_x = float(scale_x);
        render->scale_y = float(scale_y);
        widget->base.base.colour = first_colour;
        family::text_draw(&widget->base.base, nullptr, reinterpret_cast<Dword>(render));
        const auto progress = number(widget->progress);
        widget->base.base.colour = blend(progress * progress);
        render->scale_x = float(game::runtime::physics::legacy::power_retained(double(stored_x), 0.75));
        render->scale_y = float(game::runtime::physics::legacy::power_retained(double(stored_y), 0.75));
        family::text_draw(&widget->base.base, nullptr, reinterpret_cast<Dword>(render));
        widget->base.base.colour = initial_colour;
        render->scale_x = 1;
        render->scale_y = 1;
    }
    family::text_draw(&widget->base.base, nullptr, reinterpret_cast<Dword>(render));
}
} // namespace nfs::application::frontend
