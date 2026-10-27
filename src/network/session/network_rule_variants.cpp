#include "nfs/network/session/network_rules.hpp"
namespace nfs::network::packets::pending {
namespace {
Dword &setting(Dword *owner, unsigned index) {
    return **reinterpret_cast<Dword **>(bytes(owner) + 102668 + index * 4);
}
void values(Dword *owner, Dword id, unsigned count, int first = 0) {
    for (unsigned i = 0; i < count; ++i)
        at<int>(owner, 12 + i * 4) = first + int(i);
    rules::choice_values(system(96), id, count, bytes(owner) + 12);
}
void one(Dword *owner, Dword id, Dword value) {
    at<Dword>(owner, 12) = value;
    rules::choice_values(system(96), id, 1, bytes(owner) + 12);
}
void enabled(Dword id, bool value) {
    rules::choice_enabled(system(96), id, Byte(value));
}
void label(Dword id, const char *value) {
    rules::choice_label(system(96), id, value);
}
} // namespace
// Every saved rule-index and track-index switch arm.
void set_rule_variant(Dword *owner, int index, Dword value) {
    switch (index) {
    case 1: {
        setting(owner, 1) = value;
        if (at<Dword>(owner, 148))
            return;
        auto *rules_object = system(72);
        void *progress = rules::profile(rules_object, 0);
        set_rule_variant(rules_object, 2, 0);
        setting(rules_object, 13) = 1;
        void *car = rules::car_settings(rules_object, 0);
        at<Dword>(car, 0) = 1;
        at<Dword>(car, 8) = 0;
        at<Dword>(car, 16) = 0;
        at<Dword>(car, 64) = 0;
        Dword count = 0;
        const int tracks = at<int>(system(40), 4);
        for (int track = 0; track < tracks; ++track)
            for (unsigned variant = 0; variant < 4; ++variant)
                if (at<Byte>(progress, 7462 + track * 4 + variant))
                    ++count;
        const Dword selected = count ? rules::random() % count : 0;
        Dword ordinal = 0;
        for (int track = 0; track < at<int>(system(40), 4); ++track)
            for (unsigned variant = 0; variant < 4; ++variant)
                if (at<Byte>(progress, 7462 + track * 4 + variant)) {
                    if (ordinal == selected) {
                        select_track(system(40), track);
                        setting(rules_object, 6) = variant;
                    }
                    ++ordinal;
                }
        alignas(4) Byte metadata[1228];
        count = 0;
        for (int id = 0; id < 86; ++id)
            if (at<Byte>(progress, 7328 + id)) {
                rules::copy_vehicle(metadata,
                                    bytes(system(52)) + 8 + rules::catalogue_index(id) * 1228);
                if ((at<Dword>(metadata, 60) || at<Byte>(metadata, 1024)) &&
                    !at<Byte>(metadata, 32))
                    ++count;
            }
        const Dword selected_vehicle = rules::random();
        ordinal = 0;
        for (int id = 0; id < 86; ++id)
            if (at<Byte>(progress, 7328 + id)) {
                const int catalogue = rules::catalogue_index(id);
                rules::copy_vehicle(metadata, bytes(system(52)) + 8 + catalogue * 1228);
                if ((at<Dword>(metadata, 60) || at<Byte>(metadata, 1024)) &&
                    !at<Byte>(metadata, 32)) {
                    if (ordinal == selected_vehicle % count) {
                        car = rules::car_settings(rules_object, 0);
                        setting(rules_object, 22) = catalogue;
                        at<Dword>(car, 64) =
                            catalogue == id ? rules::random() % at<Dword>(metadata, 60) : 16;
                        break;
                    }
                    ++ordinal;
                }
            }
        const Dword id = setting(rules_object, 22);
        setting(rules_object, 14) = 7;
        (void)rules::catalogue_index(id);
        setting(rules_object, 15) = vehicle_class(system(52) + 1, id);
        setting(rules_object, 16) = 0;
        auto *track = at<void *>(system(40), 12 + at<Dword>(system(40), 8) * 4);
        setting(rules_object, 7) = at<Byte>(track, 24) ? 2 : 1;
        setting(rules_object, 12) = rules::random() & 3;
        setting(rules_object, 11) = 1;
        return;
    }
    case 2:
        at<Dword>(owner, 152) = value;
        rules::set_mode(owner, int(value));
        return;
    case 7:
        if (int(value) > 100)
            value = 100;
        at<Dword>(owner, 228) = value;
        setting(owner, index) = value;
        return;
    case 11: {
        if (at<Dword>(owner, 148) != 1)
            return;
        const Dword mode = at<Dword>(owner, 2484);
        at<Dword>(owner, 244) = value;
        if (mode == 0 || mode == 2) {
            enabled(16, value != 0 || setting(owner, 14) != 0);
            return;
        }
        if (mode != 1)
            return;
        enabled(16, at<Dword>(owner, 180) - at<Dword>(owner, 176) != 4294967295u || value != 0);
        return;
    }
    case 13: {
        if (at<Dword>(owner, 148) != 1)
            return;
        const Dword previous = at<Dword>(owner, 180), old_player = at<Dword>(owner, 176),
                   mode = at<Dword>(owner, 2484);
        at<Dword>(owner, 176) = value;
        if (mode == 0 || mode == 2) {
            if (value == 2) {
                at<Dword>(owner, 12) = 0;
                at<Dword>(owner, 16) = 1;
                at<Dword>(owner, 20) = 7;
                rules::choice_values(system(96), 14, 3, bytes(owner) + 12);
                return;
            }
            at<Dword>(owner, 12) = 1;
            at<Dword>(owner, 16) = 7;
            rules::choice_values(system(96), 14, 2, bytes(owner) + 12);
            if (!previous) {
                if (int(setting(owner, 15)) < 43) {
                    at<Dword>(owner, 180) = 1;
                    label(15, "<CAR>");
                } else {
                    at<Dword>(owner, 180) = 7;
                    label(15, "<OPPONENT_CARS_SINGLE_RACE>");
                }
                enabled(14, true);
                enabled(15, true);
            }
            enabled(16, true);
            enabled(11, true);
            return;
        }
        if (mode != 1)
            return;
        if (previous - value == 4294967295u) {
            enabled(15, false);
            enabled(16, setting(owner, 11) != 0);
            return;
        }
        if (previous == value) {
            label(15, "<CAR>");
            enabled(15, true);
            enabled(16, true);
            return;
        }
        enabled(16, true);
        enabled(15, true);
        if (previous == old_player) {
            label(15, "<OPPONENT_CARS_SINGLE_RACE>");
            enabled(15, true);
        }
        return;
    }
    case 14: {
        const Dword previous = at<Dword>(owner, 180), player = at<Dword>(owner, 176),
                   mode = at<Dword>(owner, 2484);
        at<Dword>(owner, 180) = value;
        if (at<Dword>(owner, 148) != 1) {
            if (value == 0 && setting(owner, 13) == 2) {
                rules::choice_values(system(96), 15, 0, bytes(owner) + 12);
                rules::choice_values(system(96), 16, 0, bytes(owner) + 12);
                return;
            }
            if (value > 1) {
                label(15, "<OPP_CLASS>");
                values(owner, 16, 3);
                if (at<Dword>(owner, 152) == 1)
                    one(owner, 7, at<Dword>(owner, 180));
                return;
            }
            label(15, "<CAR>");
            values(owner, 15, 24);
            values(owner, 16, 3);
            if (at<Dword>(owner, 152) == 1) {
                one(owner, 7, at<Dword>(owner, 180));
                return;
            }
            at<Dword>(owner, 12) = 1;
            at<Dword>(owner, 16) = 7;
            rules::choice_values(system(96), 14, 2, bytes(owner) + 12);
            return;
        }
        if (mode == 0 || mode == 2) {
            if (!value) {
                enabled(15, false);
                if (!setting(owner, 11))
                    enabled(16, false);
                return;
            }
            label(15, value == 1 ? "<CAR>" : "<OPPONENT_CARS_SINGLE_RACE>");
            enabled(15, true);
            enabled(16, true);
            return;
        }
        if (mode != 1)
            return;
        if (value - player == 4294967295u) {
            enabled(15, false);
            if (!setting(owner, 11))
                enabled(16, false);
            return;
        }
        if (value == player) {
            label(15, "<CAR>");
            enabled(15, true);
            enabled(16, true);
            return;
        }
        enabled(16, true);
        enabled(15, true);
        if (previous == player) {
            label(15, "<OPPONENT_CARS_SINGLE_RACE>");
            enabled(15, true);
        }
        return;
    }
    case 17:
        if (at<Dword>(owner, 2312) == value)
            return;
        setting(owner, index) = value;
        rules::pending::clear_language();
        rules::pending::load_language(value);
        return;
    case 34:
        setting(owner, index) = value;
        rules::pending::reload_lod(system(140));
        return;
    case 5:
        break;
    default:
        setting(owner, index) = value;
        return;
    }
    at<Dword>(owner, 220) = value;
    if (value <= 11)
        select_track(system(40), value);
    at<Dword>(owner, 12) = 0;
    at<Dword>(owner, 16) = (value == 6 || value == 7 || value == 8) ? 2 : 1;
    at<Dword>(owner, 20) = 3;
    rules::choice_values(system(96), 10, 3, bytes(owner) + 12);
}
} // namespace nfs::network::packets::pending
