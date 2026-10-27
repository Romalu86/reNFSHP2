#include "race_runtime.hpp"
namespace nfs::application::pending {

void destroy_event_nine(void *race) {
    destroy_race_state(race);
}

void destroy_event_eleven(void *race) {
    destroy_race_state(race);
}

void destroy_paired_event(void *race) {
    destroy_race_state(race);
}

void destroy_event_six(void *race) {
    destroy_race_state(race);
}

void destroy_event_base(void *race) {
    destroy_race_state(race);
}

void destroy_event_twelve(void *race) {
    destroy_race_state(race);
}

void destroy_event_five(void *race) {
    at<const RaceMethod *>(race, 0) = event_five_methods;
    destroy_race_state(race);
}

void destroy_event_thirteen(void *race) {
    destroy_race_state(race);
}

void destroy_event_one(void *race) {
    at<const RaceMethod *>(race, 0) = event_one_methods;
    destroy_race_state(race);
}

void destroy_event_seven(void *race) {
    destroy_race_state(race);
}

void destroy_event_four(void *race) {
    destroy_race_state(race);
}

void destroy_event_two(void *race) {
    at<const RaceMethod *>(race, 0) = event_two_methods;
    destroy_race_state(race);
}

void destroy_event_zero(void *race) {
    destroy_race_state(race);
}

void destroy_event_three(void *race) {
    at<const RaceMethod *>(race, 0) = event_three_methods;
    destroy_race_state(race);
}

void *__fastcall delete_event_eleven(void *race, void *, unsigned flags) {
    destroy_event_eleven(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_six(void *race, void *, unsigned flags) {
    destroy_event_six(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_base(void *race, void *, unsigned flags) {
    destroy_event_base(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_twelve(void *race, void *, unsigned flags) {
    destroy_event_twelve(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_five(void *race, void *, unsigned flags) {
    destroy_event_five(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_thirteen(void *race, void *, unsigned flags) {
    destroy_event_thirteen(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_one(void *race, void *, unsigned flags) {
    destroy_event_one(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_seven(void *race, void *, unsigned flags) {
    destroy_event_seven(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_nine(void *race, void *, unsigned flags) {
    destroy_event_nine(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_four(void *race, void *, unsigned flags) {
    destroy_event_four(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_race_state(void *race, void *, unsigned flags) {
    destroy_race_state(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_two(void *race, void *, unsigned flags) {
    destroy_event_two(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_zero(void *race, void *, unsigned flags) {
    destroy_event_zero(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_paired_event(void *race, void *, unsigned flags) {
    destroy_paired_event(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}

void *__fastcall delete_event_three(void *race, void *, unsigned flags) {
    destroy_event_three(race);
    if (flags & 1)
        gui::application_release(race);
    return race;
}
} // namespace nfs::application::pending
