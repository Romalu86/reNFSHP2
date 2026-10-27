#include "nfs/audio/speech/speech_game.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::speech::game {
using nfs::game::runtime::system;
using sound::field;
namespace {
template <class... Args> void submit_phrase(Dword phrase, Dword count, Args... args) {
    event_sink(create_event_code(0, 0, phrase), count, args...);
}
} // namespace
// The dispatch sequence and random calls follow the saved switch.
void dispatch_police_event(void *owner, Dword event, int voice, Byte, Byte, Byte, int violation,
                           int vehicle_mask, Dword speed, int units, Byte, int location) {
    int *voice_class;
    int speaker, alternate_speaker, filtered_vehicle_mask;
    Dword choice, variation, first_argument, phrase, short_phrase;
    bool below_threshold, at_threshold;
    if ((((vehicle_mask == 2048) || (vehicle_mask == 8)) || (vehicle_mask == 1048576)) ||
        ((filtered_vehicle_mask = vehicle_mask,
          vehicle_mask == 524288 &&
              (speaker = network::packets::rule_value(system(72), 17), speaker != 0)))) {
        filtered_vehicle_mask = 0;
    }
    choice = field<Dword>(system(72), 140);
    if (choice != 0) {
        return;
    }
    choice = nfs::game::runtime::emitter_random_word();
    choice = choice % 100;
    variation = nfs::game::runtime::emitter_random_word();
    speaker = graphics::unresolved::camera_state;
    variation = variation & 1;
    if (field<Dword>(owner, 20) == 0) {
        return;
    }
    if (field<int>(owner, 28) <= field<int>(owner, 24)) {
        return;
    }
    if (voice == field<int>(owner, 76)) {
        vehicle_mask =
            (int(graphics::unresolved::camera_state - field<Dword>(owner, 80)) < 1200) +
            1;
    } else {
        field<int>(owner, 76) = voice;
        vehicle_mask = 1;
    }
    field<int>(owner, 80) = speaker;
    if ((filtered_vehicle_mask == 4194304) &&
        (((speaker = dispatch_masks[voice], speaker == 32 || (speaker == 4)) ||
          (speaker == 2)))) {
        filtered_vehicle_mask = 0;
    }
    if (location == 234) {
        location = -1;
    }
    voice_class = reinterpret_cast<int *>(dispatch_masks + voice);
    if (((dispatch_masks[voice] == 4) &&
         (speaker = network::packets::rule_value(system(72), 17), speaker == 1)) &&
        ((location == 90 || (location == 40)))) {
        location = -1;
    }
    speaker = units;
    switch (event) {
    case 0:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        if (location == -1) {
            speaker = *voice_class;
            first_argument = paired_voice_masks[voice];
            phrase = 41;
        emit_vehicle_repeat:
            submit_phrase(phrase, 3, first_argument, speaker, vehicle_mask);
        } else {
            submit_phrase(42, 4, location, *voice_class, paired_voice_masks[voice], vehicle_mask);
        }
    flush_then_acknowledge:
        flush_events(0);
        goto choose_acknowledgement;
    case 1:
        field<Byte>(owner, 91) = 1;
        if ((70 < choice) && (99 < (int)speed)) {
            choice = nfs::game::runtime::emitter_random_word();
            if (50 < choice % 100) {
                submit_phrase(222, 0);
                flush_events(0);
            }
            first_argument = speed_phrase_mask(speed);
            submit_phrase(15, 3, paired_voice_masks[voice], *voice_class, first_argument);
            flush_events(0);
            choice = nfs::game::runtime::emitter_random_word();
            if (choice % 100 < 21)
                goto choose_tail;
            first_argument = voice_masks[voice];
            phrase = 1;
            choice = nfs::game::runtime::emitter_random_word();
            short_phrase = (50 < choice % 100) + 'Z';
            goto emit_short_phrase;
        }
        if ((violation == 0) || (80 < choice)) {
        announce_speed_or_vehicle:
            choice = nfs::game::runtime::emitter_random_word();
            if (50 < choice % 100) {
                submit_phrase(222, 0);
                flush_events(0);
            }
            if (69 < (int)speed) {
                vehicle_mask = speed_phrase_mask(speed);
                speaker = *voice_class;
                first_argument = paired_voice_masks[voice];
                phrase = 15;
                goto emit_vehicle_repeat;
            }
        announce_vehicle:
            submit_phrase(7, 2, paired_voice_masks[voice], *voice_class);
            goto flush_then_acknowledge;
        }
        if ((30 < choice) || (violation == 8192)) {
            if (filtered_vehicle_mask != 0) {
                choice = nfs::game::runtime::emitter_random_word();
                if (50 < choice % 100) {
                    submit_phrase(222, 0);
                    flush_events(0);
                }
                submit_phrase(17, 5, paired_voice_masks[voice], *voice_class, units, violation,
                              filtered_vehicle_mask);
                flush_events(0);
                choice = nfs::game::runtime::emitter_random_word();
                if ((50 < choice % 100) && (units != 1)) {
                    first_argument = voice_masks[voice];
                    phrase = 1;
                    short_phrase = '\r';
                    goto emit_short_phrase;
                }
                goto choose_tail;
            }
            if (violation == 8192) {
                choice = nfs::game::runtime::emitter_random_word();
                if (50 < choice % 100) {
                    submit_phrase(222, 0);
                    flush_events(0);
                }
                if (69 < (int)speed) {
                    vehicle_mask = speed_phrase_mask(speed);
                    speaker = *voice_class;
                    first_argument = paired_voice_masks[voice];
                    phrase = 15;
                    goto emit_vehicle_repeat;
                }
                goto announce_vehicle;
            }
        }
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        speaker = 1;
        if (filtered_vehicle_mask < 32769) {
            if (filtered_vehicle_mask != 32768) {
                if (filtered_vehicle_mask < 129) {
                    switch (filtered_vehicle_mask) {
                    case 0:
                    case 1:
                    case 3:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 17:
                    case 18:
                    case 19:
                    case 20:
                    case 21:
                    case 22:
                    case 23:
                    case 24:
                    case 25:
                    case 26:
                    case 27:
                    case 28:
                    case 29:
                    case 30:
                    case 31:
                    case 32:
                    case 33:
                    case 34:
                    case 35:
                    case 36:
                    case 37:
                    case 38:
                    case 39:
                    case 40:
                    case 41:
                    case 42:
                    case 43:
                    case 44:
                    case 45:
                    case 46:
                    case 47:
                    case 48:
                    case 49:
                    case 50:
                    case 51:
                    case 52:
                    case 53:
                    case 54:
                    case 55:
                    case 56:
                    case 57:
                    case 58:
                    case 59:
                    case 60:
                    case 61:
                    case 62:
                    case 63:
                        goto announce_violation;
                    default:
                        goto choose_violation_speaker;
                    }
                }
                if (filtered_vehicle_mask < 2049) {
                    if ((filtered_vehicle_mask != 2048) &&
                        ((filtered_vehicle_mask == 256 || (filtered_vehicle_mask == 512))))
                        goto choose_violation_speaker;
                } else if ((filtered_vehicle_mask == 4096) ||
                           ((filtered_vehicle_mask != 8192 && (filtered_vehicle_mask == 16384))))
                    goto choose_violation_speaker;
            }
        } else if (filtered_vehicle_mask < 8388609) {
            if ((filtered_vehicle_mask != 8388608) &&
                ((((524288 < filtered_vehicle_mask && (filtered_vehicle_mask != 1048576)) &&
                   (filtered_vehicle_mask != 2097152)) &&
                  (filtered_vehicle_mask == 4194304))))
                goto choose_violation_speaker;
        } else if (filtered_vehicle_mask < 134217729) {
            if (((filtered_vehicle_mask != 134217728) && (filtered_vehicle_mask != 16777216)) &&
                ((filtered_vehicle_mask == 33554432 || (filtered_vehicle_mask == 67108864)))) {
            choose_violation_speaker:
                choice = nfs::game::runtime::emitter_random_word();
                speaker = 2 - (Dword)(70 < choice % 100);
            }
        } else if ((filtered_vehicle_mask != 268435456) && (filtered_vehicle_mask == 536870912))
            goto choose_violation_speaker;
    announce_violation:
        submit_phrase(16, 4, paired_voice_masks[voice], *voice_class, speaker, violation);
        flush_events(0);
        choice = nfs::game::runtime::emitter_random_word();
        below_threshold = choice % 100 < 50;
        at_threshold = choice % 100 == 50;
        goto emit_acknowledgement_if_selected;
    case 2:
        if ((choice < 71) || ((int)speed < 100)) {
            if ((filtered_vehicle_mask == 0) || (violation == 0))
                goto announce_speed_or_vehicle;
            choice = nfs::game::runtime::emitter_random_word();
            if (50 < choice % 100) {
                submit_phrase(222, 0);
                flush_events(0);
            }
            submit_phrase(199, 5, *voice_class, paired_voice_masks[voice], filtered_vehicle_mask,
                          violation, vehicle_mask);
            flush_events(0);
            goto choose_acknowledgement;
        }
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        first_argument = speed_phrase_mask(speed);
        submit_phrase(15, 3, paired_voice_masks[voice], *voice_class, first_argument);
        flush_events(0);
        choice = nfs::game::runtime::emitter_random_word();
        if (choice % 100 < 21)
            goto choose_tail;
        first_argument = voice_masks[voice];
        phrase = 1;
        choice = nfs::game::runtime::emitter_random_word();
        short_phrase = (50 < choice % 100) + 'Z';
        goto emit_short_phrase;
    case 3:
        field<Byte>(owner, 91) = 1;
        variation = nfs::game::runtime::emitter_random_word();
        if (50 < variation % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        if (location == -1) {
            speaker = *voice_class;
            first_argument = paired_voice_masks[voice];
            location = 0;
        } else {
            speaker = *voice_class;
            first_argument = paired_voice_masks[voice];
        }
        submit_phrase(11, 3, first_argument, speaker, location);
        flush_events(0);
        variation = nfs::game::runtime::emitter_random_word();
        if (20 < variation % 100) {
            if (choice < 51) {
                location = voice_masks[voice];
                first_argument = 85;
            } else {
                location = voice_masks[voice];
                first_argument = 86;
            }
            goto emit_location_acknowledgement;
        }
        goto choose_tail;
    case 4:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        if (location == -1) {
            submit_phrase(172, 3, paired_voice_masks[(variation + voice)], *voice_class, units);
        } else {
            submit_phrase(35, 4, paired_voice_masks[(variation + voice)], *voice_class, units,
                          location);
        }
        flush_events(0);
        if (location != -1) {
            first_argument = 71;
            goto emit_location_acknowledgement;
        }
        first_argument = 171;
        goto emit_empty_acknowledgement;
    case 5:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        speaker = *voice_class;
        first_argument = paired_voice_masks[voice];
        phrase = 34;
        break;
    case 6:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        speaker = *voice_class;
        first_argument = paired_voice_masks[voice];
        phrase = 33;
        units = vehicle_mask;
        break;
    case 7:
        sound::streaming::stop_stream(field<Dword>(owner, 16));
        submit_phrase(206, 0);
        flush_events(0);
        submit_phrase(221, 0);
        flush_events(0);
        variation = nfs::game::runtime::emitter_random_word();
        if (50 < variation % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        if ((choice < 51) || (filtered_vehicle_mask == 0)) {
            filtered_vehicle_mask = *voice_class;
            speaker = paired_voice_masks[voice];
            first_argument = 1;
            phrase = 178;
        } else {
            speaker = *voice_class;
            first_argument = paired_voice_masks[voice];
            phrase = 188;
        }
        submit_phrase(phrase, 3, first_argument, speaker, filtered_vehicle_mask);
        flush_events(0);
        choice = nfs::game::runtime::emitter_random_word();
        if (20 < choice % 100) {
            submit_phrase(13, 1, voice_masks[voice]);
            flush_events(0);
        }
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        nfs::game::audio::state::configuration<Dword>(audio_layout::player_volume) = 0;
        nfs::game::audio::state::configuration<Dword>(audio_layout::opponent_volume) = 0;
        nfs::game::audio::apply_event_volumes();
        nfs::game::audio::stop_race_channels();
        set_engines_disabled(1);
        field<Dword>(owner, 28) = 0;
        return;
    case 8:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        speaker = *voice_class;
        first_argument = paired_voice_masks[voice];
        phrase = 63;
        units = vehicle_mask;
        break;
    case 9:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        speaker = *voice_class;
        first_argument = paired_voice_masks[voice];
        phrase = 187;
        units = vehicle_mask;
        break;
    case 10:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        if ((field<int>(owner, 56) == 2) && (location != -1)) {
            submit_phrase(56, 4, paired_voice_masks[voice], *voice_class, units, location);
            field<Dword>(owner, 56) = 0;
        } else if ((field<int>(owner, 56) == 1) && (100 < (int)speed)) {
            first_argument = speed_phrase_mask(speed);
            submit_phrase(47, 4, paired_voice_masks[voice], *voice_class, first_argument,
                          vehicle_mask);
            field<Dword>(owner, 56) = 2;
        } else {
            submit_phrase(50, 3, paired_voice_masks[voice], *voice_class, vehicle_mask);
            field<Dword>(owner, 56) = 1;
        }
        goto flush_then_tail;
    case 11:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        speaker = *voice_class;
        first_argument = paired_voice_masks[voice];
        phrase = 58;
        units = vehicle_mask;
        break;
    case 12:
        variation = nfs::game::runtime::emitter_random_word();
        if (50 < variation % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        if ((choice < 51) || (filtered_vehicle_mask == 0)) {
            vehicle_mask = *voice_class;
            speaker = paired_voice_masks[voice];
            alternate_speaker = 1;
            first_argument = 178;
        } else {
            speaker = *voice_class;
            alternate_speaker = paired_voice_masks[voice];
            first_argument = 188;
            vehicle_mask = filtered_vehicle_mask;
        }
        goto emit_three_arguments;
    case 13:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        speaker = *voice_class;
        first_argument = paired_voice_masks[voice];
        phrase = 53;
        units = vehicle_mask;
        break;
    case 14:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        first_argument = voice_masks[voice];
        phrase = 1;
        short_phrase = 202;
        goto emit_short_phrase;
    case 15:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        first_argument = 72;
        goto emit_empty_phrase;
    case 16:
        if (((location == -1) || (choice < 36)) || (field<int>(owner, 60) == location)) {
            if ((int)speed < 101) {
                if ((int)speed < 31) {
                    return;
                }
                if (field<int>(owner, 60) == 4) {
                    return;
                }
                choice = nfs::game::runtime::emitter_random_word();
                if (50 < choice % 100) {
                    submit_phrase(222, 0);
                    flush_events(0);
                }
                location = 4;
                field<Dword>(owner, 60) = 4;
                first_argument = 112;
            } else {
                choice = nfs::game::runtime::emitter_random_word();
                if (50 < choice % 100) {
                    submit_phrase(222, 0);
                    flush_events(0);
                }
                location = ((field<int>(owner, 60) != 8) - 1 & 120) + 8;
                field<int>(owner, 60) = location;
                first_argument = 112;
            }
            goto emit_location_acknowledgement;
        }
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        submit_phrase(109, 1, location);
        field<int>(owner, 60) = location;
        goto flush_then_tail;
    case 17:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        if (location != -1) {
            speaker = paired_voice_masks[voice];
            first_argument = dispatch_masks[(variation + voice)];
            phrase = 40;
            goto emit_location;
        }
        speaker = paired_voice_masks[voice];
        first_argument = dispatch_masks[(variation + voice)];
        phrase = 173;
    emit_units:
        submit_phrase(phrase, 3, first_argument, units, speaker);
        goto append_location_acknowledgement;
    case 18:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        if (location == -1) {
            first_argument = paired_voice_masks[(variation + voice)];
            phrase = 174;
            units = *voice_class;
            goto emit_units;
        }
        first_argument = paired_voice_masks[(variation + voice)];
        phrase = 38;
        units = *voice_class;
        goto emit_location;
    case 19:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        if (location == -1) {
            first_argument = paired_voice_masks[(variation + voice)];
            phrase = 175;
            units = *voice_class;
            goto emit_units;
        }
        first_argument = paired_voice_masks[(variation + voice)];
        phrase = 39;
        units = *voice_class;
    emit_location:
        submit_phrase(phrase, 4, first_argument, units, speaker, location);
    append_location_acknowledgement:
        flush_events(0);
        if (location == -1) {
            first_argument = 73;
        emit_empty_acknowledgement:
            submit_phrase(first_argument, 0);
        } else {
            first_argument = 170;
        emit_location_acknowledgement:
            submit_phrase(first_argument, 1, location);
        }
        goto flush_then_tail;
    case 20:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        speaker = *voice_class;
        first_argument = paired_voice_masks[voice];
        phrase = 37;
        break;
    case 21:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        speaker = *voice_class;
        first_argument = paired_voice_masks[voice];
        phrase = 31;
        goto emit_repeat_vehicle;
    case 22:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        first_argument = voice_masks[voice];
        phrase = 1;
        short_phrase = 201;
    emit_short_phrase:
        submit_phrase(short_phrase, phrase, first_argument);
        flush_events(0);
        goto choose_tail;
    case 23:
        first_argument = 219;
        goto emit_tail;
    case 24:
        speaker = 0;
        do {
            if ((speaker != field<int>(owner, 64)) &&
                (alternate_speaker = speaker, speaker != field<int>(owner, 68)))
                break;
            speaker = speaker + 1;
            alternate_speaker = units;
        } while (speaker < 3);
        variation = nfs::game::runtime::emitter_random_word();
        if (50 < variation % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        if (choice < 71) {
            if (choice < 26) {
                location = *voice_class;
                first_argument = 28;
                goto emit_location_acknowledgement;
            }
            speaker = *voice_class;
            alternate_speaker = paired_voice_masks[voice];
            first_argument = 23;
        } else {
            vehicle_mask = voice_masks[voice];
            speaker = dispatch_masks[alternate_speaker];
            first_argument = 25;
            alternate_speaker = paired_voice_masks[5];
        }
    emit_three_arguments:
        submit_phrase(first_argument, 3, alternate_speaker, speaker, vehicle_mask);
    flush_then_tail:
        flush_events(0);
        goto choose_tail;
    case 25:
        speaker = 0;
        do {
            if ((speaker != field<int>(owner, 64)) &&
                (alternate_speaker = speaker, speaker != field<int>(owner, 68)))
                break;
            speaker = speaker + 1;
            alternate_speaker = units;
        } while (speaker < 3);
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        units = voice_masks[voice];
        first_argument = dispatch_masks[alternate_speaker];
        phrase = 19;
        speaker = paired_voice_masks[5];
        break;
    case 26:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        first_argument = 103;
    emit_empty_phrase:
        submit_phrase(first_argument, 0);
        flush_events(0);
        goto choose_tail;
    case 27:
        first_argument = 206;
        goto emit_tail;
    case 28:
        choice = nfs::game::runtime::emitter_random_word();
        if (50 < choice % 100) {
            submit_phrase(222, 0);
            flush_events(0);
        }
        speaker = *voice_class;
        first_argument = paired_voice_masks[voice];
        phrase = 32;
    emit_repeat_vehicle:
        submit_phrase(phrase, 3, vehicle_mask, first_argument, speaker);
        flush_events(0);
    choose_acknowledgement:
        choice = nfs::game::runtime::emitter_random_word();
        below_threshold = choice % 100 < 20;
        at_threshold = choice % 100 == 20;
    emit_acknowledgement_if_selected:
        if (!below_threshold && !at_threshold) {
            first_argument = voice_masks[voice];
            phrase = 1;
            short_phrase = '\r';
            goto emit_short_phrase;
        }
        goto choose_tail;
    default:
        goto finished;
    }
    submit_phrase(phrase, 3, first_argument, speaker, units);
    flush_events(0);
choose_tail:
    choice = nfs::game::runtime::emitter_random_word();

    if (50 < choice % 100) {
        first_argument = 222;
    emit_tail:
        submit_phrase(first_argument, 0);
        flush_events(0);
    }
finished:
    return;
}
} // namespace nfs::speech::game
