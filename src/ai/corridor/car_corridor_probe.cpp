#include "car_corridor_x87_private.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
namespace nfs::game::runtime::ai::cars {
using namespace corridor_detail;

Dword __fastcall car_probe_position(void *entity, void *, Dword requested_word, Dword output_word,
                                   Dword speed_word, Dword output_speed_word, Dword horizon_word) {
    const auto *requested = reinterpret_cast<const float *>(requested_word);
    auto *output = reinterpret_cast<float *>(output_word);
    auto *output_speed = reinterpret_cast<float *>(output_speed_word);
    const float speed = float_word(speed_word);
    physics::copy_triplet(output, requested);
    *output_speed = speed;
    void *body = invoke<void *>(entities::interface_at(entity, 3), 0);
    if (!body || !at<Byte>(body, 728))
        return 0;
    if (drive_frame::legacy::route_heading_below_half(entity))
        return 0;
    alignas(4) Byte context[812];
    construct_car_corridor(context);
    struct CorridorOwner {
        Byte *context;
        ~CorridorOwner() { lifecycle::pending::destroy_population_path(context + 708); }
    } context_owner{context};
    const auto retained_speed = retained(invoke<long double>(entity, 22));
    const float current_speed = float(retained_speed);
    if ((retained_speed <= number(4.4703f)) ||
        speed <= 4.4703f) {
        return 0;
    }
    void *track = system(76);
    if (!initialize_car_corridor(context, car_physical_object(entity), horizon_word,
                                 car_path(entity), requested, speed, at<float>(track, 84396))) {
        return 0;
    }
    corridor_score_outer_road(context, at<float>(track, 84392));
    const auto scaled_penalty = number(current_speed) * -.5f;
    const float configured_penalty = at<float>(track, 84392);
    const float penalty =
        !(scaled_penalty < number(configured_penalty)) ? float(scaled_penalty) : configured_penalty;
    corridor_score_road_edges(context, penalty);
    void *mutable_body = invoke<void *>(entities::interface_at(entity, 3), 1);
    if (!at<Dword>(mutable_body, 732)) {
        for (int index = 0; index < at<int>(entity, 8); ++index) {
            void *other = at<void *>(entity, 12 + index * 4);
            if (!other)
                continue;
            const auto distance = retained(ai::pair_distance(entity, other));
            if (number(500) < distance)
                break;
            if (!at<Byte>(other, 212) || !target_on_neighboring_section(entity, other))
                continue;
            void *car_entity = invoke<void *>(entities::interface_at(other, 2), 1);
            if (car_entity) {
                void *other_body = invoke<void *>(entities::interface_at(car_entity, 3), 1);
                if (at<Dword>(other_body, 732))
                    continue;
            }
            invoke(entity, 55, context, other, speed);
        }
    }
    if (at<Dword>(system(76), 84348)) {
        const Dword kind = invoke<Dword>(entities::interface_at(entity, 1), 0);
        corridor_score_side(context, kind != 4, at<float>(track, kind == 4 ? 84392 : 84400));
    }
    const Dword result = corridor_select_target(context, output, output_speed) != 0;
    return result;
}
} // namespace nfs::game::runtime::ai::cars
