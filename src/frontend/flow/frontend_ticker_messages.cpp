#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_application_widgets.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdlib>
namespace nfs::gui::engine {
int seconds_to_ticks(float);
}
namespace nfs::application::frontend {
using game::runtime::invoke;
using gui::engine::Rectangle;
gui::Container ticker_messages;
Dword ticker_message_serial{};
static std::uint8_t registry_destroyed{};

static gui::Container *construct_ticker_registry(gui::Container *owner) {
    gui::container_construct(owner);
    return owner;
}

static void destroy_ticker_registry() {
    if (!(registry_destroyed & 1)) {
        registry_destroyed |= 1;
        gui::container_destruct(&ticker_messages);
    }
}

static void initialize_ticker_registry() {
    construct_ticker_registry(&ticker_messages);
    std::atexit(destroy_ticker_registry);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_ticker_registry, ".CRT$XCU_NFS_006900E4",
                       initialize_ticker_registry);
} // namespace

static TickerMessage *find_ticker_message(Dword id) {
    for (Dword index = 0; index < ticker_messages.count; ++index)
        if (auto *message =
                static_cast<TickerMessage *>(gui::events::container_value(&ticker_messages, index)))
            if (message->id == id)
                return message;
    return nullptr;
}

static TickerMessage *construct_ticker_message(TickerMessage *message, const gui::String *text,
                                               Dword flags, const Dword *colour, Dword reserved,
                                               int duration) {
    message->methods = gui::base_methods;
    gui::engine::construct_point(&message->position);
    try {
        gui::string_construct(&message->text);
        try {
            message->initial_colour = 0;
            message->colour = 0;
            gui::engine::construct_rectangle(&message->text_bounds);
            try {
                message->methods = ticker_message_methods;
                message->moved = 0;
                gui::string_copy(&message->text, text);
                message->flags = flags;
                message->initial_colour = *colour;
                message->duration = duration;
                message->reserved = reserved;
                message->elapsed = 0;
                message->stationary_ticks = 0;
                message->id = ++ticker_message_serial;
                gui::container_append(&ticker_messages, message, nullptr);
            } catch (...) {
                gui::engine::destroy_rectangle(&message->text_bounds);
                throw;
            }
        } catch (...) {
            gui::string_destruct(&message->text);
            throw;
        }
    } catch (...) {
        gui::engine::destroy_point(&message->position);
        throw;
    }
    return message;
}

static void destroy_ticker_message(TickerMessage *message) {
    message->methods = ticker_message_methods;
    gui::container_detach(&ticker_messages, message);
    gui::engine::destroy_rectangle(&message->text_bounds);
    gui::string_destruct(&message->text);
    gui::engine::destroy_point(&message->position);
    message->methods = gui::base_methods;
}

static TickerMessage *__fastcall delete_ticker_message(TickerMessage *message, void *, Dword flags) {
    destroy_ticker_message(message);
    if (flags & 1)
        gui::release(message);
    return message;
}
const gui::Destroy ticker_message_methods[] = {
    reinterpret_cast<gui::Destroy>(delete_ticker_message)};

static bool ticker_message_visible(TickerMessage *message, const Rectangle *bounds) {
    const Dword flags = message->flags;
    if ((flags & 40) == 40 && !(message->colour >> 24))
        return false;
    if ((flags & 24) == 24 && message->position.y > bounds->height)
        return false;
    return int(Dword(message->position.x) - Dword(message->text_bounds.width)) < bounds->width;
}

static bool can_follow_ticker_message(TickerMessage *message, TickerMessage *previous,
                                      int spacing) {
    if (!previous)
        return true;
    const Dword flags = message->flags;
    if ((flags & 136) == 136 || (flags & 72) == 72)
        return false;
    const Dword previous_flags = previous->flags;
    if (previous_flags & 8) {
        if (previous->stationary_ticks < gui::engine::seconds_to_ticks(5.0f))
            return false;
        if ((previous_flags & 24) == 24 || (previous_flags & 40) == 40)
            return false;
    }
    return int(Dword(previous->text_bounds.width) - Dword(previous->position.x) + Dword(spacing)) <= 0;
}

void initialize_ticker_message(TickerMessage *message, void *render, const Rectangle *bounds) {
    message->position.x = 0;
    Rectangle measured;
    gui::engine::construct_rectangle(&measured);
    invoke(render, 15, &message->text, &measured);
    const int height = invoke<int>(render, 17);
    message->position.y = int(Dword(bounds->height) - Dword(height)) / 2;
    message->text_bounds.height = height;
    message->text_bounds.top = measured.top;
    message->text_bounds.left = measured.left;
    message->text_bounds.width = measured.width;
    message->stationary_ticks = 0;
    message->colour = message->initial_colour;
    if ((message->flags & 136) == 136) {
        message->position.x = int(Dword(bounds->width / 2) + Dword(measured.width / 2));
        message->position.y = int(0u - Dword(message->text_bounds.height));
    }
    if ((message->flags & 72) == 72) {
        message->colour = (message->colour & 16777215) | 16777216;
        message->position.x = int(Dword(bounds->width / 2) + Dword(measured.width / 2));
    }
    gui::engine::destroy_rectangle(&measured);
}

void advance_ticker_message(TickerMessage *message, int step, const Rectangle *bounds) {
    message->moved = 0;
    message->elapsed = int(Dword(message->elapsed) + Dword(step));
    if (message->flags & 4)
        message->colour = cycle_ticker_hue(message->colour, step >> 1);
    const Dword flags = message->flags;
    if ((flags & 136) == 136 &&
        message->position.y < int(Dword(bounds->height / 2) - Dword(message->text_bounds.height / 2)))
        message->position.y = int(Dword(message->position.y) + 1);
    if ((flags & 72) == 72) {
        const int alpha = int(message->colour >> 24);
        if (alpha < int(255u - Dword(step))) {
            message->colour =
                (message->colour & 16777215) | (Dword(std::uint8_t(alpha + step)) << 24);
            return;
        }
        message->colour |= 4278190080u;
    }
    const bool centred =
        bounds->width / 2 <= int(Dword(message->position.x) - Dword(message->text_bounds.width / 2));
    if ((message->flags & 8) && centred) {
        message->stationary_ticks = int(Dword(message->stationary_ticks) + Dword(step));
        if (message->stationary_ticks < gui::engine::seconds_to_ticks(5.0f))
            return;
    }
    if ((flags & 24) == 24 && centred) {
        message->position.y = int(Dword(message->position.y) + 1);
        return;
    }
    if ((flags & 40) == 40 && centred) {
        const int alpha = int(message->colour >> 24);
        const auto next = step < alpha ? std::uint8_t(alpha - step) : std::uint8_t(0);
        message->colour = (message->colour & 16777215) | (Dword(next) << 24);
        return;
    }
    message->position.x = int(Dword(message->position.x) + Dword(step));
    message->moved = 1;
}
static Dword count_transient_messages(Ticker *ticker) {
    Dword count = 0;
    for (auto *list : {&ticker->pending, &ticker->active})
        for (Dword index = 0; index < gui::events::list_count(list); ++index)
            if (auto *message = static_cast<TickerMessage *>(gui::events::list_value(list, index)))
                if (!(message->flags & 1))
                    ++count;
    return count;
}
// Both lists contribute to the capacity check.
Dword ticker_is_full(Ticker *ticker) {
    return ticker->maximum_messages <= int(count_transient_messages(ticker));
}

Dword queue_ticker_message(Ticker *ticker, const gui::String *text, Dword flags, const Dword *colour,
                          Dword reserved, float seconds, Dword priority) {
    const int count = int(count_transient_messages(ticker));
    if (count >= ticker->maximum_messages && !priority)
        return 0;
    auto *message = static_cast<TickerMessage *>(gui::allocate(sizeof(TickerMessage)));
    if (message) {
        try {
            const int duration = gui::engine::seconds_to_ticks(seconds);
            construct_ticker_message(message, text, flags, colour, reserved, duration);
        } catch (...) {
            gui::release(message);
            throw;
        }
    }
    if (!message)
        return 0;
    const Dword result =
        priority ? gui::events::push_front(&ticker->pending, message, gui::dispose_object)
                 : gui::events::push_back(&ticker->pending, message, gui::dispose_object);
    if (result) {
        gui::destroy(reinterpret_cast<gui::Object *>(message));
        return 0;
    }
    return message->id;
}

void set_ticker_message_duration(Dword id, float seconds) {
    if (TickerMessage *message = find_ticker_message(id))
        message->duration = gui::engine::seconds_to_ticks(seconds);
}

void expire_ticker_message(Dword id) {
    if (TickerMessage *message = find_ticker_message(id)) {
        message->flags &= ~Dword(1);
        message->elapsed = message->duration;
    }
}

void clear_ticker_messages(Ticker *ticker) {
    gui::events::clear_list(&ticker->pending);
    gui::events::clear_list(&ticker->active);
}

void begin_next_ticker_message(Ticker *ticker) {
    Rectangle bounds;
    gui::widgets::client_bounds(&ticker->base.base, &bounds);
    auto *previous = static_cast<TickerMessage *>(gui::events::peek_back(&ticker->active));
    auto *message = static_cast<TickerMessage *>(gui::events::pop_front(&ticker->pending));
    if (message && (message->flags & 1))
        for (Dword index = 0; index < gui::events::list_count(&ticker->pending); ++index)
            if (auto *candidate =
                    static_cast<TickerMessage *>(gui::events::list_value(&ticker->pending, index)))
                if (!(candidate->flags & 1)) {
                    gui::events::push_front(&ticker->pending, message, gui::dispose_object);
                    gui::events::remove_value(&ticker->pending, candidate);
                    message = candidate;
                    break;
                }
    if (message) {
        if (can_follow_ticker_message(message, previous, ticker->spacing)) {
            gui::events::push_back(&ticker->active, message, gui::dispose_object);
            if (void *render = gui::active_engine()) {
                invoke(render, 14, gui::widgets::inherited_font_style(&ticker->base.base));
                initialize_ticker_message(message, render, &bounds);
            }
        } else
            gui::events::push_front(&ticker->pending, message, gui::dispose_object);
    }
    gui::engine::destroy_rectangle(&bounds);
}

void recycle_ticker_message(Ticker *ticker) {
    Rectangle bounds;
    gui::widgets::client_bounds(&ticker->base.base, &bounds);
    auto *message = static_cast<TickerMessage *>(gui::events::peek_front(&ticker->active));
    if (message && (!ticker_message_visible(message, &bounds) ||
                    (!(message->flags & 1) && message->elapsed >= message->duration))) {
        message = static_cast<TickerMessage *>(gui::events::pop_front(&ticker->active));
        if (message) {
            if ((!(message->flags & 1) && message->elapsed >= message->duration) ||
                (message->flags & 256))
                gui::destroy(reinterpret_cast<gui::Object *>(message));
            else
                gui::events::push_back(&ticker->pending, message, gui::dispose_object);
        }
    }
    gui::engine::destroy_rectangle(&bounds);
}
} // namespace nfs::application::frontend
