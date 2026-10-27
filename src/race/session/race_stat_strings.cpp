#include "race_session.hpp"
#include "../../frontend/flow/frontend_runtime.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include <cmath>
#include <climits>
#include <cstdio>
namespace nfs::network::rules::pending {
Dword uses_unicode_language();
}
namespace nfs::application::pending {
namespace {
using namespace kernel::numeric;
int rounded_clock_part(float value, Format format) {
    const auto rounded = Extended(value, format).round_to_integer();
    return rounded < INT_MIN || rounded > INT_MAX ? INT_MIN : int(rounded);
}
} // namespace

void append_race_clock(int ticks, gui::String *text) {
    const Format format = current_format();
    const Extended seconds = Extended::integer(ticks, format) * 0.01666666753590106964111328125f;
    const int minutes = int(Dword((seconds * 0.01666666753590106964111328125f).truncate_to_integer()));
    const float remainder = float(seconds - Extended::integer(int(Dword(minutes) * 60), format));
    const bool negative = remainder < 0 || std::isnan(remainder);
    const int whole = rounded_clock_part(
        float(Extended(remainder, format) + (negative ? 0.499998986721038818359375f : -0.499998986721038818359375f)), format);
    const Extended hundredths =
        (Extended(remainder, format) - Extended::integer(whole, format)) * 100.0f;
    const bool sub_negative = hundredths < Extended(0.0f, format) || std::isnan(float(hundredths));
    const int sub = rounded_clock_part(
        float(hundredths + (sub_negative ? 0.499998986721038818359375f : -0.499998986721038818359375f)), format);
    char buffer[256];
    std::sprintf(buffer, "%02d:%02d.%02d", minutes, whole, sub);
    gui::string_append_ascii(text, buffer);
}

gui::String *construct_integer_text(gui::String *destination, int value) {
    LocalFrontendString temporary;
    gui::string_format_ascii(&temporary.value, "%d", value);
    if (network::rules::pending::uses_unicode_language())
        gui::string_make_wide(&temporary.value);
    gui::string_construct_copy(destination, &temporary.value);
    return destination;
}
// The receiver of the final string copy is output+4.
void write_race_finish_text(void *race, Dword slot, void *output) {
    auto *record = participant(race, slot);
    Dword laps = at<Dword>(system(72), 228);
    if (!laps)
        laps = 1;
    const int lap = int(laps - 1);
    const int ticks = lap < 0 ? -1 : record->checkpoint_times[lap];
    LocalFrontendString text;
    if (record->retirement_tick >= 0)
        gui::string_reference(&text.value, 1249);
    else if (participant_has_finished(record))
        gui::string_reference(&text.value, 1263);
    else if (ticks < 1)
        gui::string_reference(&text.value, 1316);
    else
        append_race_clock(ticks, &text.value);
    gui::string_copy(&at<gui::String>(output, 4), &text.value);
}

void __fastcall write_race_stat_text(void *race, void *, Dword statistic, Dword slot, void *output) {
    if (!output)
        return;
    LocalFrontendString
        unused; // The function constructs and destroys this owner in every arm.
    auto *record = participant(race, slot);
    gui::String *destination = &at<gui::String>(output, 4);
    auto integer = [&](int value) {
        gui::String text;
        construct_integer_text(&text, value);
        try {
            gui::string_copy(destination, &text);
        } catch (...) {
            gui::string_destruct(&text);
            throw;
        }
        gui::string_destruct(&text);
    };
    switch (statistic) {
    case 0:
        write_race_finish_text(race, slot, output);
        break;
    case 1: {
        LocalFrontendString text;
        if (record->best_lap_ticks < 1)
            gui::string_reference(&text.value, 1316);
        else
            append_race_clock(record->best_lap_ticks, &text.value);
        gui::string_copy(destination, &text.value);
        break;
    }
    case 2:
        integer(int(record->arrests));
        break;
    case 3:
        integer(int(record->takedowns));
        break;
    case 4: {
        Dword total = 0;
        for (Dword kind = 0; kind < 5; ++kind)
            total += record->awards[kind];
        integer(int(total));
        break;
    }
    case 5: {
        LocalFrontendString text("Wins");
        gui::string_copy(destination, &text.value);
        break;
    }
    case 6:
    case 7: {
        const Dword units = network::packets::rule_value(system(72), 44);
        if (units == 0 || units == 1) {
            const float speed = statistic == 6 ? record->maximum_speed : record->average_speed;
            const auto converted =
                (Extended(speed, current_format()) * (units == 0 ? 2.2369899749755859375f : 3.599999904632568359375f))
                    .truncate_to_integer();
            integer(int(Dword(converted)));
        }
        break;
    }
    default: {
        LocalFrontendString text("HP2 STAT");
        gui::string_copy(destination, &text.value);
    }
    }
}
} // namespace nfs::application::pending
