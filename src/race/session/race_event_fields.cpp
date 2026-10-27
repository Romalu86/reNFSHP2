#include "race_session.hpp"
namespace nfs::application::pending {

Byte event_humans(void *progress) {
    return Byte((at<Dword>(progress, 0) & 256) != 0) + 1;
}

void write_event_humans(void *progress, int value) {
    const Dword flags = at<Dword>(progress, 0);
    at<Dword>(progress, 0) = (((Dword(value > 1) << 8) ^ flags) & 256) ^ flags;
}

Dword event_racers(void *progress) {
    return (at<Dword>(progress, 0) >> 9) & 7;
}

void write_event_racers(void *progress, Dword value) {
    const Dword flags = at<Dword>(progress, 0);
    at<Dword>(progress, 0) = (((value << 9) ^ flags) & 3584) ^ flags;
}

Dword event_cops(void *progress) {
    return (at<Dword>(progress, 0) >> 12) & 15;
}

void write_event_cops(void *progress, Dword value) {
    const Dword flags = at<Dword>(progress, 0);
    at<Dword>(progress, 0) = (((value << 12) ^ flags) & 61440) ^ flags;
}

Dword event_difficulty(void *progress) {
    return (at<Dword>(progress, 0) >> 20) & 15;
}

void write_event_difficulty(void *progress, Dword value) {
    const Dword flags = at<Dword>(progress, 0);
    at<Dword>(progress, 0) = (((value << 20) ^ flags) & 15728640) ^ flags;
}

Byte event_car_class(void *progress) {
    return (at<Dword>(progress, 0) >> 24) & 127;
}

void write_event_car_class(void *progress, Dword value) {
    const Dword flags = at<Dword>(progress, 0);
    at<Dword>(progress, 0) = (((value << 24) ^ flags) & 2130706432) ^ flags;
}

Byte event_traffic(void *progress) {
    return (at<Dword>(progress, 0) >> 16) & 3;
}

void write_event_traffic(void *progress, Dword value) {
    const Dword flags = at<Dword>(progress, 0);
    at<Dword>(progress, 0) = (((value << 16) ^ flags) & 196608) ^ flags;
}

Dword event_helicopters(void *progress) {
    return (at<Dword>(progress, 0) >> 18) & 3;
}

void write_event_helicopters(void *progress, Dword value) {
    const Dword flags = at<Dword>(progress, 0);
    at<Dword>(progress, 0) = (((value << 18) ^ flags) & 786432) ^ flags;
}
} // namespace nfs::application::pending

namespace nfs::application::pending {

int last_configured_lap(void *rules) {
    const Dword laps = at<Dword>(rules, 228);
    return laps ? int(laps - 1) : 0;
}
} // namespace nfs::application::pending
