#include "ai_car_behavior.hpp"
#include "../../police/pursuit/chase_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace nfs::application::pending {
void *traffic_actor_view(void *);
}
namespace nfs::game::runtime::ai::cars {
using kernel::numeric::Extended;

void __fastcall racer_body_contact(void *entity, void *, Dword contact_word) {
    void *contact = reinterpret_cast<void *>(contact_word);
    void *body = at<void *>(contact, 0);
    if (body && physics::vehicle_car(body)) {
        void *car = physics::vehicle_car(body);
        if (void *other = at<void *>(car, 7988))
            if (void *traffic = application::pending::traffic_actor_view(other))
                if (at<Byte>(traffic, 576))
                    at<Byte>(entity, 580) = 1;
        if (audio::vehicle::car_audio_suppressed(car))
            car_body_contact(entity, nullptr, contact_word);
    }
}

void __fastcall player_body_contact(void *entity, void *, Dword contact_word) {
    racer_body_contact(entity, nullptr, contact_word);
    if ((at<int>(entity, 632) > 0 && at<Dword>(entity, 628)) || invoke<Dword>(entity, 50))
        return;
    void *contact = reinterpret_cast<void *>(contact_word);
    if (at<Dword>(contact, 48) != 2 || !at<void *>(contact, 0))
        return;
    void *car = physics::vehicle_car(at<void *>(contact, 0));
    if (!car || !audio::vehicle::car_audio_suppressed(car))
        return;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto impact = (E(at<float>(contact, 4)) / E(at<float>(contact, 56)) - 1.f) * 0.05f;
    const float strength = float(impact);
    if (!(E(0) < impact))
        return;
    const float applied = strength < 1.f ? strength : 1.f;
    invoke(entity, 49, applied);
    void *traits = bytes(entities::interface_at(entity, 5)) + 8;
    Dword flags;
    switch (at<Dword>(traits, 0)) {
    case 1:
        flags = 7;
        break;
    case 2:
        flags = 3;
        break;
    case 4:
        flags = 81;
        break;
    case 5:
        flags = 88;
        break;
    default:
        flags = 19;
        break;
    }
    switch (at<Dword>(system(72), 192)) {
    case 0:
        flags = (flags & 4294967175u) | 3;
        break;
    case 1:
        flags = (flags & 4294967255u) | 2;
        break;
    case 2:
        flags &= 4294967291u;
        break;
    }
    if (at<Dword>(entity, 600) == 3)
        flags &= 4294967175u;
    if (flags & 64) {
        if (const Dword other = invoke<Dword>(at<void *>(car, 7988), 11)) {
            at<Dword>(entity, 628) = other;
            impact = E(strength);
            if (at<Dword>(traits, 0) != 5)
                impact = impact * 0.5f;
            at<float>(entity, 636) = float(impact + at<float>(entity, 636));
        }
    }
}

void __fastcall ai_cop_body_contact(void *entity, void *, Dword contact_word) {
    player_body_contact(entity, nullptr, contact_word);
    void *perpetrator = entities::interface_at(entity, 4);
    void *chase = at<void *>(perpetrator, 8);
    if (!chase || !at<Dword>(chase, 40))
        return;
    void *contact = reinterpret_cast<void *>(contact_word);
    if (at<Dword>(contact, 48) != 2 || !at<void *>(contact, 0))
        return;
    void *car = physics::vehicle_car(at<void *>(contact, 0));
    void *other = car ? at<void *>(car, 7988) : nullptr;
    if (!other || other != (at<Dword>(chase, 40) ? at<void *>(chase, 28) : nullptr))
        return;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    const float strength = float(E(at<float>(contact, 4)) / E(at<float>(contact, 56)));
    void *tuning = system(76);
    const float low = at<float>(tuning, 84596), high = at<float>(tuning, 84600);
    // TEST AH,41 / JNP rejects less and equal, but admits unordered values.
    if (strength <= low)
        return;
    if (strength >= high)
        at<float>(entity, 660) = 1.f;
    else {
        const auto accumulated = (E(strength) - low) / (E(high) - low) + at<float>(entity, 660);
        at<float>(entity, 660) = float(accumulated);
        if (E(1) < accumulated)
            at<float>(entity, 660) = 1.f;
    }
    if (at<float>(entity, 660) >= 1.f)
        entities::pending::set_police_chase_outcome(at<void *>(perpetrator, 8), 1);
}

void __fastcall autonomous_cop_body_contact(void *entity, void *, Dword contact_word) {
    if (at<Byte>(entity, 576) || invoke<Dword>(entity, 50))
        return;
    void *contact = reinterpret_cast<void *>(contact_word);
    if (!at<void *>(contact, 0))
        return;
    void *car = physics::vehicle_car(at<void *>(contact, 0));
    if (!car || !at<void *>(car, 7988))
        return;
    void *racer = invoke<void *>(entities::interface_at(at<void *>(car, 7988), 2), 2);
    if (!racer)
        return;
    void *chase = at<void *>(entities::interface_at(entity, 4), 8);
    if (!lifecycle::pending::population_loading) {
        if (!chase)
            return;
    } else if (!chase) {
        if (!recently_stunned(racer, 1))
            invoke(entity, 57, racer, Dword{1});
        return;
    }
    if (lifecycle::pending::cop_can_be_stunned && at<void *>(chase, 44) == racer) {
        const auto format = kernel::numeric::current_format();
        auto E = [&](float value) { return Extended(value, format); };
        const auto impact = (E(at<float>(contact, 4)) / E(at<float>(contact, 56)) - 3.f) * 0.05f;
        const float strength = float(impact);
        if (E(0) < impact)
            invoke(entity, 49, strength < 1.f ? strength : 1.f);
    }
}

void __fastcall traffic_body_contact(void *entity, void *, Dword contact_word) {
    void *contact = reinterpret_cast<void *>(contact_word);
    if (!at<void *>(contact, 0))
        return;
    void *car = physics::vehicle_car(at<void *>(contact, 0));
    if (!car || traffic_entity(at<void *>(car, 7988)))
        return;
    const auto format = kernel::numeric::current_format();
    const auto strength =
        Extended(at<float>(contact, 4), format) / Extended(at<float>(contact, 56), format);
    if (Extended(4.470300197601318359375f, format) < strength) {
        const Dword delay = Dword((strength * -22.3698635101318359375f).truncate_to_integer());
        const Dword deadline = graphics::unresolved::camera_state - delay;
        if (at<int>(entity, 584) <= std::int32_t(deadline))
            at<Dword>(entity, 584) = deadline;
    }
    car_body_contact(entity, nullptr, contact_word);
}
} // namespace nfs::game::runtime::ai::cars
