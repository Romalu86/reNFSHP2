#include "race_runtime.hpp"
namespace nfs::application::pending {
// Unwritten fields deliberately retain their storage contents.
void *construct_participant(void *storage) {
    auto *record = static_cast<volatile RaceParticipant *>(storage);
    constexpr Dword unset_speed_bits = 3212836864u;
    at<volatile Dword>(storage, offsetof(RaceParticipant, maximum_speed)) = unset_speed_bits;
    at<volatile Dword>(storage, offsetof(RaceParticipant, average_speed)) = unset_speed_bits;
    record->identifier = -1;
    record->best_lap_ticks = -1;
    record->last_checkpoint = -1;
    record->completed_checkpoints = 0;
    record->awarded_medal = -1;
    record->takedowns = 0;
    record->arrests = 0;
    record->losses = 0;
    record->checkpoint_time = -1;
    gui::string_construct(
        reinterpret_cast<gui::String *>(bytes(storage) + offsetof(RaceParticipant, name)));
    record->finish_position = -1;
    record->retirement_tick = -1;
    record->best_time = 0;
    record->finish_time = -1;
    record->disabled = 0;
    record->retired = 0;
    record->disconnected = 0;
    record->kind = 0;
    record->methods = network::world::pending::race_player_methods;
    for (unsigned i = 0; i < 100; ++i) {
        record->checkpoint_times[i] = -1;
        record->checkpoint_positions[i] = -1;
    }
    for (unsigned i = 0; i < 5; ++i) {
        record->awards[i] = 0;
        record->award_times[i] = 0;
    }
    return storage;
}

void reset_participant(void *storage) {
    // Preserve the individual DWORD and BYTE reset stores, including their order.
    auto *record = static_cast<volatile RaceParticipant *>(storage);
    constexpr Dword unset_speed_bits = 3212836864u;
    at<volatile Dword>(storage, offsetof(RaceParticipant, average_speed)) = unset_speed_bits;
    at<volatile Dword>(storage, offsetof(RaceParticipant, maximum_speed)) = unset_speed_bits;
    record->best_lap_ticks = -1;
    record->completed_checkpoints = 0;
    record->awarded_medal = -1;
    record->checkpoint_time = -1;
    record->finish_position = -1;
    record->retirement_tick = -1;
    record->takedowns = 0;
    record->arrests = 0;
    record->losses = 0;
    record->best_time = 0;
    record->last_checkpoint = -1;
    for (auto &value : record->counters)
        value = 0;
    record->finish_time = -1;
    record->disabled = 0;
    record->retired = 0;
    record->disconnected = 0;
    for (unsigned i = 0; i < 100; ++i) {
        record->checkpoint_times[i] = -1;
        record->checkpoint_positions[i] = -1;
    }
    for (unsigned i = 0; i < 5; ++i) {
        record->awards[i] = 0;
        record->award_times[i] = 0;
    }
}
// Assignment preserves the receiver's table and owns its String.
void *assign_participant(void *destination, const void *source) {
    if (destination == source)
        return destination;
    const Dword completed = at<Dword>(source, 16);
    const Dword medal = at<Dword>(source, 20);
    const Dword best_lap = at<Dword>(source, 8);
    at<Dword>(destination, 16) = completed;
    const Dword average_speed = at<Dword>(source, 48);
    at<Dword>(destination, 20) = medal;
    const Dword checkpoint_time = at<Dword>(source, 52);
    at<Dword>(destination, 8) = best_lap;
    const Dword maximum_speed = at<Dword>(source, 44);
    at<Dword>(destination, 48) = average_speed;
    const Dword finish_time = at<Dword>(source, 936);
    at<Dword>(destination, 52) = checkpoint_time;
    const Dword retirement_tick = at<Dword>(source, 884);
    at<Dword>(destination, 44) = maximum_speed;
    const Dword finish_position = at<Dword>(source, 880);
    at<Dword>(destination, 936) = finish_time;
    const Dword arrests = at<Dword>(source, 32);
    at<Dword>(destination, 884) = retirement_tick;
    const Dword losses = at<Dword>(source, 36);
    at<Dword>(destination, 880) = finish_position;
    const Dword takedowns = at<Dword>(source, 28);
    at<Dword>(destination, 32) = arrests;
    const Dword checkpoint = at<Dword>(source, 12);
    at<Dword>(destination, 36) = losses;
    const Byte disabled = at<Byte>(source, 940);
    at<Dword>(destination, 28) = takedowns;
    const Dword best_time = at<Dword>(source, 888);
    at<Dword>(destination, 12) = checkpoint;
    const Byte disconnected = at<Byte>(source, 942);
    at<Byte>(destination, 940) = disabled;
    at<Dword>(destination, 888) = best_time;
    const Byte retired = at<Byte>(source, 941);
    at<Byte>(destination, 942) = disconnected;
    at<Byte>(destination, 941) = retired;
    gui::string_copy(reinterpret_cast<gui::String *>(bytes(destination) + 860),
                     reinterpret_cast<const gui::String *>(bytes(source) + 860));
    at<Dword>(destination, 944) = at<Dword>(source, 944);
    at<Dword>(destination, 4) = at<Dword>(source, 4);
    for (unsigned i = 0; i < 100; ++i) {
        at<Dword>(destination, 56 + i * 4) = at<Dword>(source, 56 + i * 4);
        at<Dword>(destination, 456 + i * 4) = at<Dword>(source, 456 + i * 4);
    }
    for (unsigned i = 0; i < 10; ++i)
        at<Dword>(destination, 892 + i * 4) = at<Dword>(source, 892 + i * 4);
    for (unsigned i = 0; i < 5; ++i) {
        at<Dword>(destination, 948 + i * 4) = at<Dword>(source, 948 + i * 4);
        at<Dword>(destination, 968 + i * 4) = at<Dword>(source, 968 + i * 4);
    }
    return destination;
}

void record_takedown(void *record) {
    ++at<Dword>(record, 28);
}

void record_arrest(void *record) {
    const int limit = at<int>(system(72), 2264);
    const Dword count = at<Dword>(record, 32) + 1;
    at<Dword>(record, 32) = count;
    if (limit < static_cast<int>(count))
        at<Dword>(record, 884) = game::race_ticks;
}

Byte participant_completed_checkpoint(void *record, int checkpoint) {
    return checkpoint < at<int>(record, 16);
}

int participant_checkpoint_duration(void *record, int checkpoint) {
    if (checkpoint < 0)
        return -1;
    if (checkpoint == 0)
        return at<int>(record, 56);
    return static_cast<int>(at<Dword>(record, 56 + Dword(checkpoint) * 4) -
                            at<Dword>(record, 52 + Dword(checkpoint) * 4));
}

void *participant_read_view(void *race, Dword slot) {
    return participant(race, slot);
}

void *participant_write_view(void *race, Dword slot) {
    return participant(race, slot);
}

Byte participant_is_cop(void *race, Dword slot) {
    const int kind = participant(race, slot)->kind;
    return kind == 3 || kind == 2;
}

Byte participant_blocked(void *race, Dword slot) {
    return participant(race, slot)->disabled;
}

Byte participant_disconnected(void *race, Dword slot) {
    return participant(race, slot)->disconnected;
}
} // namespace nfs::application::pending
