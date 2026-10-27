#include "arrest_cinematic_runtime.hpp"
#include "nfs/core/application/application.hpp"
#include "../../race/rules/race_rules_x87.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

Byte can_finish_chase(void *chase) {
    if (void *target = at<void *>(chase, 44))
        if (invoke<Dword>(entities::interface_at(target, 1), 0) != 5 && !invoke<Byte>(target, 47))
            return 0;
    return 1;
}

Dword car_speech_mask(int kind) {
    const auto *entry = dependencies::car_names;
    do {
        if (entry->model == kind)
            return entry->flags;
        ++entry;
    } while (entry->model != -1);
    return 0;
}

void end_arrest_cinematic(void *owner, void *chase) {
    if (at<int>(system(72), 176) == 1) {
        void *target = at<void *>(chase, 44);
        void *race = at<void *>(system(92), 4);
        if (participant_enabled(race, network::world::vehicle_slot(target))) {
            race = at<void *>(system(92), 4);
            const Dword participant = network::world::vehicle_slot(target);
            set_participant_enabled(race, participant, 0);
        }
    }
    void *car = at<void *>(at<void *>(chase, 44), 368);
    world::camera::end_arrest_camera(tick_services[at<Dword>(car, 1776)]);
    finish_chase(owner, chase);
    const Dword return_to_frontend = at<Dword>(system(72), 140);
    state<Byte>(7353316) = 0;
    if (return_to_frontend)
        at<Dword>(system(96), 36) = 23;
}
// Cdecl callback; its context contains chase and owner pointers.
void __cdecl arrest_cinematic_complete(void *context) {
    if (state<Byte>(7352728))
        end_arrest_cinematic(at<void *>(context, 4), at<void *>(context, 0));
}

int choose_arrest_marker(void *) {
    const int count = at<int>(system(24), 68);
    float player_position[3];
    std::memcpy(player_position, invoke<const float *>(at<void *>(system(76), 84164), 0), 12);
    player_position[2] = -player_position[2];
    int selected = -1;
    bool selected_ground = false;
    float best_distance;
    auto *boundary = static_cast<Dword *>(track::boundary(system(36), 1, 0));
    for (int index = 0; index < count; ++index) {
        float position[3];
        std::memcpy(position, runtime::level_marker(system(24), 2, index), 12);
        const float distance = float(runtime::hardware::distance_squared(player_position, position));
        if (selected_ground && best_distance <= distance)
            continue;
        bool grounded = false;
        if (boundary && distance < 4000000.f) {
            position[1] = float(static_cast<long double>(position[1]) + 10.f);
            alignas(4) Byte contact[82]{};
            physics::clear_ground_contact(contact);
            void *cell = track::spatial_cell(boundary, position);
            grounded = physics::contacts_pending::search_ground_cell(cell, boundary, position,
                                                                     contact, 0) != 0;
        }
        if ((!selected_ground && (selected < 0 || grounded || !(best_distance <= distance))) ||
            (selected_ground && grounded && !(best_distance <= distance))) {
            best_distance = distance;
            selected = index;
            selected_ground = grounded;
        }
    }
    return selected;
}

void complete_police_chase(Dword *owner, void *chase, int player) {
    if (at<Dword>(chase, 40))
        if (void *cop = at<void *>(chase, 28))
            application::pending::traffic_actor_view(cop);
    if (!can_finish_chase(chase))
        return;
    const int mode = at<int>(system(72), 152);
    at<Dword>(chase, 4) = 0;
    int listener;
    if (mode == 10 || mode == 4) {
        listener = pursuit_listener(chase);
        if (listener == -1)
            listener = 0;
    } else
        listener = at<int>(system(84), 68);
    void *target = at<void *>(chase, 44);
    if (void *vehicle = invoke<void *>(target, 9))
        physics::request_car_reset(at<void *>(vehicle, 368), saved_float(1098536505));
    if (invoke<Dword>(target, 11) && at<int>(system(72), 176) <= 1) {
        void *characters = system(48);
        void *context = &state<Byte>(7352476 + Dword(player) * 8);
        at<void *>(context, 4) = owner;
        at<void *>(context, 0) = chase;
        listener = player_target_listener(target);
        if (at<int>(system(72), 176) == 1)
            at<Byte>(system(84), 89) = 0;
        speech::game::dispatch_police_event(system(84), 7, listener, 1, 0, 0, 0,
                                            int(car_speech_mask(at<int>(target, 356))), 0, 0, 0,
                                            0);
        const int marker = choose_arrest_marker(owner);
        network::world::Iterator iterator;
        network::world::construct_iterator(&iterator, state<Dword *>(7353312));
        Dword entity = network::world::first(&iterator);
        while (!network::world::iterator_done(&iterator)) {
            void *object = reinterpret_cast<void *>(entity);
            if (!invoke<Dword>(entities::interface_at(object, 1), 0))
                invoke(object, 6, 0);
            entity = network::world::next(&iterator);
        }
        world::actor_resources::begin_arrest_cinematic(
            characters, marker, reinterpret_cast<Dword>(arrest_cinematic_complete), context);
        state<Byte>(7353316) = 1;
        application::pending::stop_force_feedback();
        at<Dword>(chase, 88) = 4;
        return;
    }
    const int players = at<int>(system(72), 176), race_mode = at<int>(system(72), 152);
    if (players == 2 && (race_mode == 10 || race_mode == 4))
        speech::game::dispatch_police_event(system(84), 12, listener, 0, 0, 0, 0,
                                            int(car_speech_mask(at<int>(target, 356))), 0, 0, 0,
                                            0);
    else if (players == 1 && !at<Byte>(system(84), 89))
        speech::game::dispatch_police_event(system(84), 12, listener, 0, 0, 0, 0,
                                            int(car_speech_mask(at<int>(target, 356))), 0, 0, 0,
                                            0);
    else if (invoke<Dword>(target, 11))
        speech::game::dispatch_police_event(system(84), 27, listener, 0, 0, 0, 0,
                                            int(car_speech_mask(at<int>(target, 356))), 0, 0, 0,
                                            0);
    finish_chase(owner, chase);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
