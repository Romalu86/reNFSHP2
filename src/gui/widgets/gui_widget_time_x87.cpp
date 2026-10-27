#include "../lifecycle/gui_timer_x87.hpp"
#include "gui_widget_shared_methods.hpp"
#include "../../frontend/flow/frontend_application_widgets.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstring>

namespace nfs::gui::timer_x87 {
namespace {
long double seconds_value(int ticks) {
    alignas(16) unsigned char bytes[10];
    ticks_to_seconds_native80(ticks, bytes);
    long double value;
    std::memcpy(&value, bytes, 10);
    return value;
}
}

// The F32 elapsed store does not replace the retained comparison.
void dialog_tick(void *object, std::uint32_t ticks) {
    using game::runtime::at;
    auto *widget = static_cast<widgets::Widget *>(object);
    const int elapsed_ticks = int(ticks) < 2 ? 1 : int(ticks);
    const long double elapsed = seconds_value(elapsed_ticks) + at<float>(widget, 284);
    at<float>(widget, 284) = static_cast<float>(elapsed);
    const float timeout = at<float>(widget, 280);
    if (timeout != 0.0f && elapsed >= static_cast<long double>(timeout) &&
        !widget->modal_result)
        events::post_widget(widget, 24, 8, widget->id);
}


void ticker_timer(void *object, std::uint32_t id) {
    auto *ticker = static_cast<application::frontend::Ticker *>(object);
    if (id == ticker->timer_id)
        ticker->accumulated_seconds = static_cast<float>(
            static_cast<long double>(0.0333333350718021392822265625f) + ticker->accumulated_seconds);
}

// Each FISTP consumes the preceding stored F32 value.
void update_ticker(void *object, std::uint32_t ticks) {
    using namespace application::frontend;
    auto *ticker = static_cast<Ticker *>(object);
    if (ticker->accumulated_seconds > 0.0f) {
        const float elapsed = static_cast<float>(seconds_value(int(ticks)));
        ticker->accumulated_seconds = static_cast<float>(
            static_cast<long double>(ticker->accumulated_seconds) - elapsed);
        engine::Rectangle bounds;
        widgets::client_bounds(&ticker->base.base, &bounds);
        try {
            const float half_speed = static_cast<float>(
                static_cast<long double>(ticker->speed) * 0.5f);
            const int maximum = game::runtime::hardware::current_integer_f32(half_speed);
            const float scaled_step = static_cast<float>(
                (static_cast<long double>(elapsed) / 0.0333333350718021392822265625f) * ticker->speed);
            const int rounded_step = game::runtime::hardware::current_integer_f32(scaled_step);
            const int step = rounded_step < 1 ? 1 :
                             rounded_step > maximum ? maximum : rounded_step;
            for (Dword index = 0; index < events::list_count(&ticker->active); ++index)
                if (auto *message = static_cast<TickerMessage *>(
                        events::list_value(&ticker->active, index)))
                    advance_ticker_message(message, step, &bounds);
        } catch (...) {
            engine::destroy_rectangle(&bounds);
            throw;
        }
        engine::destroy_rectangle(&bounds);
    }
    begin_next_ticker_message(ticker);
    recycle_ticker_message(ticker);
}
}
