#include "frontend_profile_controls.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "nfs/audio/music/music_clients.hpp"

namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
namespace hardware = game::runtime::hardware;
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Profile statistics require the native x87 intermediate format");

static Dword profile_car_id(void *catalogue, Dword index) {
    return at<Dword>(catalogue, 4 + index * 1228);
}

Dword profile_car_label(void *catalogue, Dword id) {
    for (int index = 0; index < at<int>(catalogue, 0); ++index)
        if (at<Dword>(catalogue, 4 + Dword(index) * 1228) == id)
            return at<Dword>(catalogue, 24 + Dword(index) * 1228);
    return Dword(-1);
}

Dword count_profile_results(void *preferences, Dword kind, Dword result) {
    Dword count = 0;
    for (Dword index = 0; index < 33; ++index)
        if (at<Dword>(preferences, (kind == 2 ? 8180 : 28772) + index * 624) == result)
            ++count;
    return count;
}
static void append_statistic(DriverProfilePage *page, const gui::String *text) {
    family::ListItem item;
    construct_toggle_item(&item, text, 0);
    try {
        invoke(&page->statistics->items, 8, &item);
    } catch (...) {
        gui::string_destruct(&item.tooltip);
        gui::string_destruct(&item.text);
        throw;
    }
    gui::string_destruct(&item.tooltip);
    gui::string_destruct(&item.text);
}
template <class... Arguments>
static void append_reference_statistic(DriverProfilePage *page, gui::String *text, Dword reference,
                                       Arguments... arguments) {
    gui::string_reference(text, reference);
    gui::string_format(text, text, arguments...);
    append_statistic(page, text);
}
static bool first_speed_unit() {
    return network::packets::rule_value(static_cast<Dword *>(system(72)), 44) == 0;
}
static long double converted_profile_speed(float speed, bool first) {
    return static_cast<long double>(speed) *
           (first ? 2.2369899749755859375f : 3.599999904632568359375f);
}

static Dword truncate_profile_value(long double value) {
    return static_cast<Dword>(hardware::truncate_integer_native80(&value));
}

void format_profile_speed(gui::String *text, float speed) {
    const bool first = first_speed_unit();
    const float converted = float(converted_profile_speed(speed, first));
    gui::string_append_reference(text, first ? 362 : 363);
    gui::string_format(
        text, text,
        truncate_profile_value(static_cast<long double>(converted)));
}

void append_profile_car_speed(DriverProfilePage *page, Dword index, float speed) {
    void *catalogue = bytes(page->events) + 4;
    pending::LocalFrontendString label, value;
    gui::string_reference(&label.value,
                          profile_car_label(catalogue, profile_car_id(catalogue, index)));
    gui::string_reference(&value.value, 722);
    gui::string_append_ascii(&value.value, ": %d");
    format_profile_speed(&value.value, speed);
    gui::string_append(&label.value, &value.value);
    if (page->statistics)
        append_statistic(page, &label.value);
}
static void append_overview_speed(DriverProfilePage *page, gui::String *text, Dword reference,
                                  float speed) {
    const bool first = first_speed_unit();
    {
        pending::LocalFrontendString format;
        gui::string_reference(&format.value, reference);
        gui::string_format(text, &format.value,
                           truncate_profile_value(converted_profile_speed(speed, first)));
    }
    gui::string_append_ascii(text, " ");
    gui::string_append_reference(text, first ? 362 : 363);
    append_statistic(page, text);
}

void append_profile_overview(DriverProfilePage *page) {
    pending::LocalFrontendString text(
        &lifecycle::construction::unresolved::shared_string);
    append_reference_statistic(page, &text.value, 386, at<Dword>(page->preferences, 244),
                               at<Dword>(page->preferences, 164));
    append_reference_statistic(page, &text.value, 1118, at<Dword>(page->preferences, 308),
                               at<Dword>(page->preferences, 172));
    append_reference_statistic(page, &text.value, 388, at<Dword>(page->preferences, 276),
                               at<Dword>(page->preferences, 168));
    append_reference_statistic(page, &text.value, 1126, at<Dword>(page->preferences, 7324));
    append_reference_statistic(page, &text.value, 389, count_profile_results(page->preferences, 2, 5));
    append_reference_statistic(page, &text.value, 391, count_profile_results(page->preferences, 4, 5));
    append_reference_statistic(page, &text.value, 1727, at<Dword>(page->preferences, 7556));
    append_reference_statistic(page, &text.value, 392, at<Dword>(page->preferences, 5236));
    append_reference_statistic(page, &text.value, 394, at<Dword>(page->preferences, 6716));
    append_overview_speed(page, &text.value, 1124, at<float>(page->preferences, 6320));
    const int duration = at<int>(page->preferences, 6724);
    float average = 0;
    if (duration > 0)
        average = float(
            static_cast<long double>(at<float>(page->preferences, 7552)) /
            static_cast<long double>(duration / 60));
    append_overview_speed(page, &text.value, 1125, average);
}

void refresh_profile_statistics(DriverProfilePage *page) {
    page->preferences = bytes(system(72)) + 2488;
    const Dword category = page->category ? selected_toggle_value(page->category) : 0;
    if (page->statistics) {
        pending::LocalFrontendString text(
            &lifecycle::construction::unresolved::shared_string);
        invoke(&page->statistics->items, 1);
        switch (category) {
        case 0:
            append_profile_overview(page);
            break;
        case 1:
        case 2: {
            const Dword begin = category == 1 ? 5288 : 6372;
            for (Dword index = 0; index < 38; ++index) {
                const float speed = at<float>(page->preferences, begin + 4 * index);
                if (!(speed <= 0))
                    append_profile_car_speed(page, index, speed);
            }
            break;
        }
        case 3:
        case 4:
            for (int index = 0; index < at<int>(page->cars, 4); ++index) {
                const float speed = at<float>(page->preferences,
                                              (category == 3 ? 5240 : 6324) + 4 * Dword(index));
                if (!(speed <= 0)) {
                    pending::LocalFrontendString label, value;
                    gui::string_reference(
                        &label.value, at<Dword>(at<void *>(page->cars, 12 + 4 * Dword(index)), 12));
                    gui::string_reference(&value.value, 722);
                    gui::string_append_ascii(&value.value, ": %d");
                    format_profile_speed(&value.value, speed);
                    gui::string_append(&label.value, &value.value);
                    if (page->statistics)
                        append_statistic(page, &label.value);
                }
            }
            break;
        case 5:
        case 6: {
            append_reference_statistic(page, &text.value, category == 5 ? 1806 : 1807,
                                       at<Dword>(page->preferences, category == 5 ? 228 : 236));
            const Dword kind = category == 5 ? 2 : 4;
            append_reference_statistic(page, &text.value, 1735,
                                       count_profile_results(page->preferences, kind, 5));
            append_reference_statistic(page, &text.value, 1736,
                                       count_profile_results(page->preferences, kind, 4));
            append_reference_statistic(page, &text.value, 1737,
                                       count_profile_results(page->preferences, kind, 3));
            break;
        }
        case 7:
            append_reference_statistic(page, &text.value, 1739, at<Dword>(page->preferences, 692),
                                       at<Dword>(page->preferences, 704));
            append_reference_statistic(page, &text.value, 1740, at<Dword>(page->preferences, 696),
                                       at<Dword>(page->preferences, 708));
            append_reference_statistic(page, &text.value, 1741, at<Dword>(page->preferences, 700),
                                       at<Dword>(page->preferences, 712));
            break;
        }
        if (!at<Dword>(page->statistics, 280)) {
            gui::string_reference(&text.value, 38);
            append_statistic(page, &text.value);
        }
    }
    using namespace game::audio;
    const Dword volume =
        Dword(game::runtime::race_audio::group_volume(state::configuration<Dword>(audio_layout::music_volume), 4, 127));
    sound::lock();
    game::runtime::race_audio::fade_stream_volume(state::library().stream, 500, volume);
    sound::unlock();
}
} // namespace nfs::application::frontend
