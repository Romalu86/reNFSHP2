#include "nfs/network/session/network_player_state.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::network::players {
namespace {
struct ScopedString {
    gui::String value;
    ScopedString() {
        gui::string_construct(&value);
    }
    ~ScopedString() {
        gui::string_destruct(&value);
    }
};
} // namespace
// The by-value String is owned and consumed by this callee.
void notify_game(Dword *owner, gui::String text, int sender, int prefix) {
    try {
        if (auto *frontend = field<Dword *>(owner, 92))
            ui::notice(frontend, nullptr, &text, static_cast<std::int16_t>(sender), prefix, 0, 0);
    } catch (...) {
        gui::string_destruct(&text);
        throw;
    }
    gui::string_destruct(&text);
}

void apply_state(Dword *queue, const void *packet) {
    ui::PlayerRecord player;
    ui::initialize_car_settings(&player.car);
    auto *bytes = static_cast<const std::uint8_t *>(packet);
    const Dword key = reinterpret_cast<Dword>(bytes + 2);
    namespace q = lifecycle::construction::network::queue;
    if (!q::copy_matching(queue, key, reinterpret_cast<Dword>(&player), nullptr))
        return;
    ScopedString line;
    gui::string_copy(&line.value, &lifecycle::construction::unresolved::shared_string);
    const auto previous_slot = player.slot;
    const Dword previous_vehicle = field<Dword>(&player.car, 60);
    player.slot = field<std::int16_t>(const_cast<void *>(packet), 6);
    player.phase = field<std::int16_t>(const_cast<void *>(packet), 4);
    kernel::memory_copy(&player.car, bytes + 8, 84);
    q::replace(queue, key, reinterpret_cast<Dword>(&player), nullptr);
    if (previous_slot == player.slot && previous_vehicle == field<Dword>(&player.car, 60))
        return;
    ScopedString name;
    gui::string_assign(&name.value, player.name);
    gui::string_make_wide(&name.value);
    if (player.slot == -1) {
        ScopedString pattern;
        gui::string_reference(&pattern.value, 1352);
        gui::string_format(&line.value, &pattern.value, gui::string_data(&name.value));
    } else {
        ScopedString vehicle;
        gui::string_assign(&vehicle.value,
                           vehicle_name(static_cast<int>(field<Dword>(&player.car, 60))));
        gui::string_make_wide(&vehicle.value);
        ScopedString pattern;
        gui::string_reference(&pattern.value, 1353);
        const void *car_text = gui::string_data(&vehicle.value);
        const void *player_text = gui::string_data(&name.value);
        gui::string_format(&line.value, &pattern.value, player_text, car_text);
    }
    gui::String notification;
    gui::string_construct_copy(&notification, &line.value);
    notify_game(network_game(), notification, -1, 0);
}
} // namespace nfs::network::players
