#include "frontend_network_pages.hpp"
#include "../../race/session/race_runtime.hpp"
#include "../../gui/hud/hud_display.hpp"
#include "../../ai/driving/ai_car_behavior.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::cars {

Dword participant_best_lap(void *manager, Dword slot) {
    return at<Dword>(at<void *>(manager, 4), 592 + slot * 988);
}
} // namespace nfs::game::runtime::ai::cars
namespace nfs::application::frontend {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
namespace {
void copy_reference(gui::String *out, Dword reference) {
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    gui::string_copy(out, &text.value);
}
void copy_time(gui::String *out, Dword ticks) {
    gui::String text;
    hud::construct_time_text(&text, ticks, 2);
    try {
        gui::string_copy(out, &text);
    } catch (...) {
        gui::string_destruct(&text);
        throw;
    }
    gui::string_destruct(&text);
}
void append_speed(gui::String *out, long double value, Dword units) {
    const auto converted = Dword(static_cast<std::int64_t>(value));
    std::int32_t speed;
    std::memcpy(&speed, &converted, sizeof(speed));
    gui::string_format_ascii(out, "%d", speed);
    gui::string_append_ascii(out, " ");
    gui::string_append_reference(out, units ? 363 : 362);
}
} // namespace

Dword format_network_position(int slot, gui::String *text, int *position) {
    auto *manager = system(92);
    int value;
    if (at<std::int16_t>(system(104), 8) == 6) {
        void *race = at<void *>(manager, 8);
        void *record = bytes(race) + 584 + Dword(slot) * 988;
        if (at<Dword>(at<void *>(manager, 8), 34620) == 1 &&
            std::uint8_t(network::race_pending(static_cast<Dword *>(record))))
            value = int(invoke<Dword>(at<void *>(system(92), 4), 24) - at<Dword>(record, 880));
        else {
            int checkpoint;
            if (at<Dword>(at<void *>(system(92), 8), 34620) == 1)
                checkpoint = at<int>(record, 12);
            else {
                checkpoint = int(at<Dword>(system(72), 228) - 1);
                if (checkpoint < 0)
                    checkpoint = 0;
            }
            value = checkpoint < 0 ? -1 : at<int>(record, 456 + Dword(checkpoint) * 4);
        }
    } else {
        void *record = bytes(at<void *>(manager, 4)) + 584 + Dword(slot) * 988;
        if (at<Dword>(at<void *>(manager, 8), 34620) == 1 &&
            std::uint8_t(network::race_pending(static_cast<Dword *>(record)))) {
            *position = int(at<Dword>(system(72), 168) - at<Dword>(record, 880));
            copy_reference(text, 1263);
            return 1;
        }
        value = int(invoke<Dword>(at<void *>(system(92), 4), 42, slot) - 1);
    }
    *position = value;
    if (value < 0) {
        gui::string_assign(text, "-");
        *position = 98;
    } else
        copy_reference(text, Dword(value) + 283);
    return 1;
}

void format_network_statistic(const network::ui::PlayerRecord *player, int column,
                              gui::String *text) {
    auto *record = const_cast<network::ui::PlayerRecord *>(player);
    gui::string_assign(text, "-");
    const int slot = player->slot;
    switch (column) {
    case 0:
        if (player->phase == 9 && slot >= 0) {
            if (invoke<std::uint8_t>(at<void *>(system(92), 4), 29, slot)) {
                gui::string_format_ascii(text, "%d",
                                         game::runtime::ai::cars::participant_completed_checkpoints(
                                             system(92), Dword(slot)) +
                                             1);
                return;
            }
            if (at<Dword>(at<void *>(system(92), 4), 34620) == 1 &&
                std::uint8_t(network::race_pending(
                    reinterpret_cast<Dword *>(bytes(at<void *>(system(92), 4)) +
                                              584 + Dword(slot) * 988))))
                gui::string_reference(text, 463);
            else
                gui::string_reference(text, 1696);
        }
        break;
    case 2: {
        pending::LocalFrontendString format;
        gui::string_reference(&format.value, 1604);
        gui::string_format(text, &format.value, at<Dword>(record, 132));
        break;
    }
    case 3:
        if (player->phase == 9 && slot >= 0) {
            void *race_record = bytes(at<void *>(system(92), 8)) + 584 + Dword(slot) * 988;
            const int checkpoint = int(at<Dword>(race_record, 16) - 1);
            if (checkpoint >= 0) {
                const int ticks = at<int>(race_record, 56 + Dword(checkpoint) * 4);
                if (ticks > 0 && !invoke<std::uint8_t>(at<void *>(system(92), 4), 29, slot) &&
                    (at<Dword>(at<void *>(system(92), 4), 34620) != 1 ||
                     !std::uint8_t(network::race_pending(static_cast<Dword *>(race_record)))))
                    copy_time(text, Dword(ticks));
            }
        }
        break;
    case 4:
        if (player->phase == 9 && slot >= 0) {
            const int ticks =
                int(game::runtime::ai::cars::participant_best_lap(system(92), Dword(slot)));
            if (ticks > 0)
                copy_time(text, Dword(ticks));
        }
        break;
    case 5:
    case 6:
        if (player->phase == 9 && slot >= 0)
            if (void *car = at<void *>(system(68), 20 + Dword(slot) * 4)) {
                const Dword units = network::packets::rule_value(system(72), 44);
                const long double value =
                    static_cast<long double>(at<float>(car, column == 5 ? 1852 : 1856)) *
                    (units ? 3.599999904632568359375f : 2.2369899749755859375f);
                if (column == 6 || 0 < value)
                    append_speed(text, value, units);
            }
        break;
    case 7:
    case 8: {
        pending::LocalFrontendString format;
        gui::string_reference(&format.value, 1625);
        gui::string_format(text, &format.value,
                           Dword(at<std::uint16_t>(record, 126)) + at<std::uint16_t>(record, 122),
                           Dword(at<std::uint16_t>(record, 128)) + at<std::uint16_t>(record, 124));
        break;
    }
    default:
        if (player->phase != 9)
            gui::string_assign(text, "-");
        else if (slot >= 0 && at<int>(record, 92) >= 0)
            gui::string_assign(text, network::players::vehicle_name(at<int>(record, 92)));
        else
            gui::string_reference(text, 1697);
        break;
    }
}
} // namespace nfs::application::frontend
