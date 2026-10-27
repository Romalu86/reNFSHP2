#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/physics/contacts/contact_dependencies.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cmath>
#include <initializer_list>
namespace nfs::game::runtime::physics {

void dispatch_track_contacts(void *list) {
    for (void *item = at<void *>(list, 4); item; item = at<void *>(item, 12))
        invoke(static_cast<Byte *>(item) - 16, 3);
}

void begin_track_contacts(Dword *owner) {
    if (void *list = at<void *>(owner, 92))
        dispatch_track_contacts(list);
}

void clear_car_skids(Dword *car) {
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        if (at<Byte>(car, 1695 + wheel)) {
            void *&strip = at<void *>(car, 2240 + wheel * 296);
            if (strip)
                particles::lifecycle::mark_skid_finished(strip);
            strip = nullptr;
            at<Byte>(car, 1695 + wheel) = 0;
            at<Byte>(car, 1716 + wheel) = 0;
        }
    }
    for (unsigned offset : {1692, 1693, 1694})
        if (at<Byte>(car, offset))
            at<Byte>(car, offset) = 0;
}

void reset_car_motion(Dword *car) {
    void *body = at<void *>(car, 1768);
    for (unsigned offset : {1804, 1808, 1812})
        at<Dword>(car, offset) = 1203982336;
    for (unsigned offset : {20, 24, 28})
        at<Dword>(body, offset) = 0;
    for (unsigned offset : {3196, 3200, 3204, 3184, 3188, 3192, 3148, 3152, 3156, 1800})
        at<Dword>(car, offset) = 0;
    kernel::memory_zero(bytes(body) + 604, 72);
    at<std::uint16_t>(body, 596) = 1;
    at<Dword>(body, 608) = at<Dword>(body, 612) = at<Dword>(body, 616) = at<Dword>(body, 620) =
        0;
    at<Dword>(body, 604) = 0;
    at<Dword>(body, 732) = at<Dword>(system(68), 692);
    contacts_pending::clear_car_controls(car);
    const Byte selected_gear = static_cast<Byte>(contact::request_next_gear(car));
    at<Dword>(car, 7996) = 0;
    at<Dword>(car, 8000) = 0;
    at<Byte>(car, 1582) = selected_gear;
}

void reset_car(Dword *car) {
    ++at<Dword>(car, 1832);
    void *body = at<void *>(car, 1768);
    const float upright = at<float>(body, 112);
    if (upright <= 0.75f || std::isnan(upright) || !(at<float>(body, 20) < 1.0f) ||
        !(at<float>(body, 28) < 1.0f) || !at<Dword>(body, 532) ||
        at<std::uint16_t>(body, 596) > 5) {
        contacts_pending::find_reset_transform(car);
        body = at<void *>(car, 1768);
        at<Dword>(body, 8) = at<Dword>(body, 12) = at<Dword>(body, 16) = 0;
        reset_car_motion(car);
        const Dword kind = at<Dword>(system(68), 696);
        at<Dword>(at<void *>(car, 1768), 732) = at<Dword>(system(68), 692);
        at<Dword>(car, 1800) = kind;
    }
}

void update_car_reset(Dword *car) {
    int reason = 0;
    void *interface = at<void *>(car, 7988);
    if (race_started && !at<Dword>(car, 1800) && static_cast<int>(game::race_ticks) > 599 &&
        ((!interface || (!invoke<Dword>(interface, 11) && !ai::racer(interface))) ||
         !(at<float>(car, 3168) > 26.82180023193359375f)) &&
        at<float>(at<void *>(car, 1768), 112) < 0.5f)
        reason = 1;
    void *body = at<void *>(car, 1768);
    if (!at<Dword>(body, 532))
        reason = 2;
    else if (!reason)
        at<Dword>(car, 1796) = 0;
    if (reason && !at<Dword>(car, 1796))
        at<Dword>(car, 1796) = 96;
    interface = at<void *>(car, 7988);
    if (interface && invoke<Dword>(interface, 9)) {
        void *racer = invoke<void *>(interface, 9);
        void *base = bytes(racer) + at<int>(at<void *>(racer, 4), 16);
        if (at<Dword>(base, 12))
            return;
    }
    if (!(at<float>(body, 592) > 0.89406001567840576171875f) &&
        !(kernel::numeric::Extended(0.100000001490116119384765625f, kernel::numeric::current_format()) <
          retained_vector_length(reinterpret_cast<float *>(bytes(body) + 20))) &&
        at<Dword>(car, 1796)) {
        const int remaining = static_cast<int>(--at<Dword>(car, 1796));
        if (remaining < 1 || reason == 2)
            reset_car(car);
    }
}

void update_car_effects(Dword *car) {
    contacts_pending::update_collision_feedback(car);
    contacts_pending::update_contact_particles(car);
    contacts_pending::update_body_damage(car);
    contacts_pending::update_exhaust_effects(car);
    contacts_pending::update_ground_effects(car);
}

void reset_track_vehicle(void *binding) {
    void *body = at<void *>(binding, 80);
    if (!body)
        return;
    if (at<float>(body, 60) == 0)
        at<Byte>(binding, 116) = 1;
    const float z = -at<float>(body, 688);
    const Dword x = at<Dword>(body, 680), y = at<Dword>(body, 684);
    at<float>(binding, 36) = z;
    at<Dword>(binding, 32) = y;
    at<Dword>(binding, 28) = x;
    body = at<void *>(binding, 80);
    at<Dword>(binding, 40) = at<Dword>(body, 692);
    at<Dword>(binding, 44) = at<Dword>(body, 696);
    at<float>(binding, 48) = -at<float>(body, 700);
    at<Dword>(binding, 52) = at<Dword>(body, 704);
    at<Dword>(binding, 56) = at<Dword>(body, 708);
    at<float>(binding, 60) = -at<float>(body, 712);
    at<float>(binding, 64) = -at<float>(body, 716);
    at<float>(binding, 68) = -at<float>(body, 720);
    at<Dword>(binding, 72) = at<Dword>(body, 724);
    release_vehicle(body);
    at<void *>(binding, 80) = nullptr;
}
} // namespace nfs::game::runtime::physics
