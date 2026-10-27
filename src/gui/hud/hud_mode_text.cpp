#include "hud_display.hpp"
#include "../../ai/driving/ai_car_behavior.hpp"
#include "nfs/network/session/network_player_state.hpp"
namespace nfs::network {
Dword race_pending(Dword *);
}
namespace nfs::network::rules::pending {
Dword uses_unicode_language();
}
namespace nfs::application::pending {
void *race_participant_owner_view(void *, Dword);
}
namespace nfs::application::frontend::hud {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using pending::LocalFrontendString;
static void *active_race() {
    return at<void *>(system(92), 4);
}
static void copy_integer(gui::String *output, Dword number) {
    gui::String temporary;
    pending::construct_integer_text(&temporary, int(number));
    try {
        gui::string_copy(output, &temporary);
    } catch (...) {
        gui::string_destruct(&temporary);
        throw;
    }
    gui::string_destruct(&temporary);
}
static void copy_time(gui::String *output, Dword ticks, int precision) {
    gui::String temporary;
    construct_time_text(&temporary, ticks, precision);
    try {
        gui::string_copy(output, &temporary);
    } catch (...) {
        gui::string_destruct(&temporary);
        throw;
    }
    gui::string_destruct(&temporary);
}

static void __cdecl arrests_value(Text *, void *car, gui::String *output) {
    void *race = active_race();
    const Dword slot = at<Dword>(car, 1772);
    const auto *record = pending::participant(race, slot);
    copy_integer(output,
                 pending::participant_is_cop(race, slot) ? record->takedowns : record->arrests);
}

static void __cdecl arrests_label(Text *, void *car, gui::String *output) {
    gui::string_reference(
        output, pending::participant_is_cop(active_race(), at<Dword>(car, 1772)) ? 779 : 780);
}

static void __cdecl elapsed_value(Text *, void *car, gui::String *output) {
    copy_time(output, invoke<Dword>(active_race(), 40, at<Dword>(car, 1772)), 2);
}

static void __cdecl elapsed_label(Text *, void *, gui::String *output) {
    gui::string_reference(output, 468);
}

static void __cdecl quota_value(Text *, void *car, gui::String *output) {
    const Dword amount = at<Dword>(active_race(), 34488 + at<Dword>(car, 1772) * 4);
    LocalFrontendString format;
    gui::string_reference(&format.value, 775);
    gui::string_format(output, &format.value, amount,
                       network::packets::rule_value(system(72), 8));
}

static void __cdecl quota_label(Text *, void *, gui::String *output) {
    gui::string_reference(output, 1247);
}

static void __cdecl lap_value(Text *, void *car, gui::String *output) {
    void *rules = system(72);
    const int configured_laps = at<int>(rules, 228);
    int current = 1, total = 1;
    if (configured_laps > 1) {
        const Dword completed = game::runtime::ai::cars::participant_completed_checkpoints(
            system(92), at<Dword>(car, 1772));
        total = at<int>(system(72), 228);
        if (network::session_active() && at<Dword>(rules, 152) == 1) {
            for (int slot = 0; slot < at<int>(rules, 168); ++slot) {
                void *record = pending::race_participant_owner_view(system(92), Dword(slot));
                if (record && !Byte(network::race_pending(static_cast<Dword *>(record))) &&
                    (at<Byte>(record, 942) || at<Byte>(record, 940) ||
                     at<Byte>(record, 941) || at<int>(record, 884) > -1 ||
                     pending::participant_is_cop(active_race(), Dword(slot))))
                    --total;
            }
            if (total < 1)
                total = 1;
            if (total > configured_laps)
                total = configured_laps;
        }
        current = int(completed + 1);
        if (current < 1)
            current = 1;
        else if (current > total)
            current = total;
    }
    LocalFrontendString format;
    gui::string_reference(&format.value, 775);
    gui::string_format(output, &format.value, current, total);
}

static void __cdecl lap_label(Text *, void *, gui::String *output) {
    gui::string_reference(output, 254);
}

static void __cdecl lap_clock_value(Text *, void *car, gui::String *output) {
    copy_time(output, invoke<Dword>(active_race(), 39, at<Dword>(car, 1772)), 2);
}

static void __cdecl lap_clock_label(Text *, void *, gui::String *output) {
    gui::string_reference(output, 468);
}

static void __cdecl best_lap_value(Text *widget, void *car, gui::String *output) {
    if (at<int>(system(72), 228) > 1) {
        const int ticks = pending::participant(active_race(), at<Dword>(car, 1772))->best_lap_ticks;
        copy_time(output, Dword(ticks < 1 ? 0 : ticks), 2);
        if (widget->base.base.base.value_84 != Dword(ticks) && ticks) {
            invoke(widget, 48);
            widget->base.base.base.value_84 = Dword(ticks);
        }
    }
}

static void __cdecl best_lap_label(Text *widget, void *car, gui::String *output) {
    if (at<int>(system(72), 228) > 1) {
        gui::string_reference(output, 1256);
        const int ticks = pending::participant(active_race(), at<Dword>(car, 1772))->best_lap_ticks;
        if (widget->base.base.base.value_84 != Dword(ticks) && ticks) {
            invoke(widget, 48);
            widget->base.base.base.value_84 = Dword(ticks);
        }
    }
}

static void __cdecl remaining_value(Text *, void *car, gui::String *output) {
    copy_time(output, invoke<Dword>(active_race(), 41, at<Dword>(car, 1772)), 2);
}

static void __cdecl remaining_label(Text *, void *, gui::String *output) {
    gui::string_reference(output, 638);
}
using TextWriter = void(__cdecl *)(Text *, void *, gui::String *);
struct ModeText {
    int mode, field;
    TextWriter label, value;
};
static const ModeText mode_text[] = {{0, 0, lap_label, lap_value},
                                     {0, 1, lap_clock_label, lap_clock_value},
                                     {0, 2, best_lap_label, best_lap_value},
                                     {1, 0, lap_label, lap_value},
                                     {1, 1, remaining_label, remaining_value},
                                     {1, 2, best_lap_label, best_lap_value},
                                     {2, 0, lap_label, lap_value},
                                     {2, 1, lap_clock_label, lap_clock_value},
                                     {2, 2, best_lap_label, best_lap_value},
                                     {10, 0, elapsed_label, elapsed_value},
                                     {10, 1, arrests_label, arrests_value},
                                     {4, 0, elapsed_label, elapsed_value},
                                     {4, 1, arrests_label, arrests_value},
                                     {9, 0, elapsed_label, elapsed_value},
                                     {9, 1, arrests_label, arrests_value},
                                     {7, 0, lap_label, lap_value},
                                     {7, 1, elapsed_label, elapsed_value},
                                     {5, 0, remaining_label, remaining_value},
                                     {5, 1, best_lap_label, best_lap_value},
                                     {3, 0, lap_label, lap_value},
                                     {3, 1, remaining_label, remaining_value},
                                     {3, 2, best_lap_label, best_lap_value},
                                     {6, 0, quota_label, quota_value},
                                     {6, 1, elapsed_label, elapsed_value},
                                     {12, 0, elapsed_label, elapsed_value},
                                     {13, 0, elapsed_label, elapsed_value},
                                     {14, 0, nullptr, nullptr}};

void write_mode_text(Text *widget, Dword field, void *car, gui::String *value, gui::String *label) {
    const int mode = at<int>(system(72), 152);
    for (auto *entry = mode_text; entry->mode < 14; ++entry) {
        if (entry->mode == mode && Dword(entry->field) == field) {
            if (label)
                entry->label(widget, car, label);
            if (value)
                entry->value(widget, car, value);
            return;
        }
    }
}

Dword support_usage(const void *record, int kind) {
    return kind >= 0 && kind < 10 ? at<Dword>(record, 892 + Dword(kind) * 4) : Dword(-1);
}

gui::String *append_localized_text(gui::String *output, const gui::String *source, Dword reference) {
    LocalFrontendString text;
    gui::string_copy(&text.value, source);
    gui::string_append_reference(&text.value, reference);
    return gui::string_construct_copy(output, &text.value);
}

gui::String *construct_time_text(gui::String *output, Dword ticks, int precision) {
    LocalFrontendString text;
    const Dword language = network::packets::rule_value(system(72), 17);
    const char *minutes = ":", *seconds = ".";
    switch (language) {
    case 1:
        minutes = "'";
        seconds = "\"";
        break;
    case 2:
    case 4:
    case 5:
    case 6:
        minutes = seconds = ":";
        break;
    case 3:
        minutes = "'.";
        seconds = "\".";
        break;
    }
    const Dword sign = Dword(std::int32_t(ticks) >> 31);
    const int absolute = int((ticks ^ sign) - sign);
    const int total_seconds = absolute / 60, whole_minutes = total_seconds / 60;
    const int remainder = total_seconds % 60;
    if (precision == 0)
        gui::string_format_ascii(&text.value, "%d%s00", whole_minutes, minutes);
    else if (precision == 1)
        gui::string_format_ascii(&text.value, "%d%s%02d%s00", whole_minutes, minutes, remainder,
                                 seconds);
    else if (precision == 2)
        gui::string_format_ascii(&text.value, "%d%s%02d%s%02d", whole_minutes, minutes, remainder,
                                 seconds, ((absolute % 60) * 100) / 60);
    if (network::rules::pending::uses_unicode_language())
        gui::string_make_wide(&text.value);
    return gui::string_construct_copy(output, &text.value);
}
} // namespace nfs::application::frontend::hud
