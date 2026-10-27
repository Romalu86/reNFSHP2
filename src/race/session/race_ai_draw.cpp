#include "race_frame_draw.hpp"
#include "../../vehicles/rendering/vehicle_renderable_frame.hpp"
namespace nfs::game::runtime::ai::cars {
Byte camera_mode_flashes(void *);
Dword cop_unavailable(void *);
}
namespace nfs::game::world::frame {
// Const racer queries use the same virtual base.
static void *racer_view(void *entity) {
    return runtime::ai::kind(entity) == 2 ? entity : nullptr;
}
static void *entity_car(void *entity) {
    const auto *offsets = at<const int *>(entity, 4);
    return invoke<void *>(offset(entity, 4 + offsets[2]), 1);
}
// Incoming this is not read by the entry.
void prepare_ai_vehicles(void *, void *director) {
    if (at<Byte>(subsystem(48), 104))
        return;
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, network::world::vehicle_registry);
    for (Dword value = network::world::first(&iterator); value;
         value = network::world::next(&iterator)) {
        void *entity = reinterpret_cast<void *>(value);
        if (!at<Byte>(entity, 212) || invoke<Byte>(entity, 7, director))
            continue;
        void *vehicle = entity_car(entity);
        if (!vehicle)
            continue;
        void *car = at<void *>(vehicle, 368);
        void *body = at<void *>(car, 1768);
        float position[3] = {at<float>(body, 680), at<float>(body, 684), -at<float>(body, 688)};
        body = at<void *>(car, 1768);
        float rotation[9] = {at<float>(body, 692), at<float>(body, 696), -at<float>(body, 700),
                              at<float>(body, 704), at<float>(body, 708), -at<float>(body, 712),
                              -at<float>(body, 716), -at<float>(body, 720), at<float>(body, 724)};
        void *renderable = at<void *>(car, 3372);
        vehicle_render::bind_renderable_car(renderable, car);
        vehicle_render::prepare_renderable_frame(renderable, director, position, rotation);
    }
}
// Incoming this is not read by the entry.
void draw_ai_vehicle_lights(void *, void *director) {
    if (at<Byte>(subsystem(48), 104))
        return;
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, network::world::vehicle_registry);
    for (Dword value = network::world::first(&iterator); value;
         value = network::world::next(&iterator)) {
        void *entity = reinterpret_cast<void *>(value);
        if (!at<Byte>(entity, 212) || invoke<Byte>(entity, 7, director))
            continue;
        void *vehicle = entity_car(entity);
        if (!vehicle)
            continue;
        void *car = at<void *>(vehicle, 368), *renderable = at<void *>(car, 3372);
        vehicle_render::bind_renderable_car(renderable, car);
        vehicle_render::draw_renderable_lights(renderable, director);
    }
    Dword remaining = at<Dword>(subsystem(72), 168);
    for (Dword offset_bytes = 20; remaining; --remaining, offset_bytes += 4) {
        void *car = at<void *>(subsystem(68), offset_bytes);
        if (car && runtime::physics::car_is_remote(car)) {
            void *renderable = at<void *>(car, 3372);
            vehicle_render::bind_renderable_car(renderable, car);
            vehicle_render::draw_renderable_lights(renderable, director);
        }
    }
}
// Incoming this is not read by the entry.
void draw_pursuit_overlays(void *, void *director) {
    if (!at<void *>(director, 300) || !runtime::ai::cars::camera_mode_flashes(director))
        return;
    void *entity = at<void *>(at<void *>(director, 300), 7988);
    if (!entity)
        return;
    void *racer = racer_view(entity);
    if (!racer)
        return;
    network::world::Iterator iterator;
    network::world::construct_iterator(&iterator, network::world::vehicle_registry);
    for (Dword value = network::world::first(&iterator); value;
         value = network::world::next(&iterator)) {
        entity = reinterpret_cast<void *>(value);
        if (!at<Byte>(entity, 212))
            continue;
        void *cop = runtime::ai::police(entity);
        if (!cop || Byte(runtime::ai::cars::cop_unavailable(cop)))
            continue;
        const auto *racer_offsets = at<const int *>(racer, 4);
        void *chase = at<void *>(racer, 12 + racer_offsets[4]);
        Byte selected = 0;
        if (chase) {
            if (at<void *>(chase, 44) != cop)
                continue;
            selected = 1;
        } else {
            const auto *cop_offsets = at<const int *>(cop, 4);
            if (at<void *>(cop, 12 + cop_offsets[4]))
                continue;
        }
        invoke<void>(cop, 56, director, Dword(selected));
    }
}
} // namespace nfs::game::world::frame
