#include "../session/race_session.hpp"
namespace nfs::application::pending {
Dword event_grid_snapshot[12]{};
Dword event_result_snapshot[12]{};

void reset_event_progress(void *progress) {
    at<Dword>(progress, 0) = (at<Dword>(progress, 0) & 4294967055u) | 1;
    kernel::memory_zero(bytes(progress) + 308, 192);
    for (unsigned i = 0; i < 12; ++i)
        at<Dword>(progress, 548 + i * 4) = 0;
}

void advance_event_progress(void *progress) {
    // The inactive path reads only the flag byte.
    if (!(at<volatile Byte>(progress, 0) & 1))
        return;
    Dword flags = at<volatile Dword>(progress, 0);
    flags = ((((flags & 4294967280u) + 16) ^ flags) & 240) ^ flags;
    const Dword last = (flags >> 1) & 7;
    at<volatile Dword>(progress, 0) = flags;
    if (last + 1 < ((flags >> 4) & 15))
        at<volatile Dword>(progress, 0) = ((last + 2) * 16) ^ (flags & 4294967054u);
}

Dword event_stage(void *progress) {
    return (at<Dword>(progress, 0) >> 4) & 15;
}

bool event_complete_progress(void *progress) {
    const Dword flags = at<Dword>(progress, 0);
    return ((flags >> 4) & 15) == ((flags >> 1) & 7) + 2;
}

Dword event_active(void *progress) {
    return at<Dword>(progress, 0) & 1;
}

Dword event_points(void *progress, Dword slot) {
    Dword score = 0;
    const Dword stages = event_stage(progress);
    for (Dword i = 0; i < stages; ++i) {
        const Byte position = at<Byte>(progress, 308 + slot + i * 24);
        if (position < 8)
            score += 8 - position;
    }
    return score;
}

void read_event_track(void *progress, Dword stage, Dword *track) {
    const auto packed = at<volatile std::uint16_t>(progress, 4 + stage * 2);
    volatile Dword *destination = track;
    destination[0] = packed & 31;
    destination[1] = (packed >> 5) & 7;
    const Byte settings = at<volatile Byte>(progress, 5 + stage * 2);
    destination[3] = 1;
    destination[2] = settings & 15;
    destination[4] = (packed >> 12) & 1;
    destination[5] = (packed >> 13) & 1;
}

void save_event_grid(void *progress) {
    for (unsigned i = 0; i < 12; ++i)
        event_grid_snapshot[i] = at<Dword>(progress, 500 + i * 4);
    for (unsigned i = 0; i < 12; ++i)
        event_result_snapshot[i] = at<Dword>(progress, 548 + i * 4);
}

void restore_event_grid(void *progress) {
    for (unsigned i = 0; i < 12; ++i)
        at<Dword>(progress, 500 + i * 4) = event_grid_snapshot[i];
    for (unsigned i = 0; i < 12; ++i)
        at<Dword>(progress, 548 + i * 4) = event_result_snapshot[i];
}

void read_event_participant(void *progress, Dword slot, void *entry) {
    const Dword packed = at<volatile Dword>(progress, 20 + slot * 12);
    at<volatile Dword>(entry, 0) = packed & 1;
    at<volatile Dword>(entry, 8) = (packed >> 2) & 1;
    at<volatile Dword>(entry, 12) = (packed >> 3) & 1;
    at<volatile Dword>(entry, 16) = (packed >> 4) & 1;
    at<volatile Dword>(entry, 68) = (packed >> 5) & 1;
    at<volatile Dword>(entry, 60) = (packed >> 12) & 63;
    // Snapshot both raw fields before a possibly overlapping grid write.
    const Dword configuration = at<volatile Dword>(progress, 28 + slot * 12);
    const Dword variant = at<volatile Dword>(progress, 24 + slot * 12);
    at<volatile Dword>(entry, 52) = (packed >> 6) & 63;
    at<volatile Dword>(entry, 56) = configuration;
    at<volatile Dword>(entry, 64) = variant;
}

void write_event_participant(void *progress, Dword slot, const void *entry) {
    Dword packed = at<volatile Dword>(progress, 20 + slot * 12);
    packed = ((at<volatile Dword>(entry, 0) ^ packed) & 1) ^ packed;
    for (unsigned bit = 1; bit <= 5; ++bit)
        packed = (((at<volatile Dword>(entry, bit == 5 ? 68 : bit * 4) << bit) ^ packed) &
                  (1u << bit)) ^ packed;
    const Dword grid_choice = at<volatile Byte>(entry, 52) & 63;
    packed = (grid_choice << 6) ^ (packed & 4294963263u);
    const Dword car_choice = at<volatile Byte>(entry, 60) & 63;
    packed = (car_choice << 12) ^ (packed & 4294709247u);
    at<volatile Dword>(progress, 28 + slot * 12) = at<volatile Dword>(entry, 56);
    const Dword starting_position = at<volatile Dword>(entry, 20);
    const Dword variant = at<volatile Dword>(entry, 64);
    at<volatile Dword>(progress, 20 + slot * 12) =
        (((starting_position << 18) ^ packed) & 1835008) ^ packed;
    at<volatile Dword>(progress, 24 + slot * 12) = variant;
}

void record_event_position(void *progress, Dword stage, Dword slot, Byte position) {
    at<Byte>(progress, 308 + stage * 24 + slot) = position;
}

Byte event_track(void *progress, Dword stage) {
    return at<Byte>(progress, 4 + stage * 2) & 31;
}
} // namespace nfs::application::pending
