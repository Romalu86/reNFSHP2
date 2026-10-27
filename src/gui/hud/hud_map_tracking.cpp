#include "hud_map.hpp"
#include "../../network/session/network_pursuit_runtime.hpp"
#include <cmath>
namespace nfs::application::frontend::hud {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using kernel::numeric::Extended;
static void *perpetrator_interface(void *entity) {
    return game::runtime::ai::entities::interface_at(entity, 2);
}
Extended filtered_map_value(const float *samples) {
    const auto format = kernel::numeric::current_format();
    const auto middle = Extended(samples[2], format) + samples[1];
    return (middle + middle + samples[3] + samples[0]) * 0.16666667163372039794921875f;
}
// All current map consumers test AL.
Dword map_camera_visible(void *camera) {
    const Dword mode = at<Dword>(camera, 312 + at<Dword>(camera, 356) * 4);
    if (!at<Byte>(camera, 172) && mode <= 3 && !at<Byte>(camera, 164) &&
        !at<Byte>(camera, 165) && !at<Byte>(camera, 166))
        return (mode & 4294967040u) | 1;
    return mode & 4294967040u;
}

Dword map_camera_hidden(int player) {
    void *camera = game::tick_services[player];
    return camera && !Byte(map_camera_visible(camera));
}

Byte map_car_is_racer(void *car) {
    void *entity = at<void *>(car, 7988);
    return entity && invoke<void *>(perpetrator_interface(entity), 2);
}

Extended map_car_heading(Map *map) {
    const auto format = kernel::numeric::current_format();
    if (!(map->style & 1))
        return Extended(0, format);
    void *car = game::runtime::physics::local_car(system(68), 0);
    void *body = at<void *>(car, 1768);
    const float x = map->reversed ? -at<float>(body, 700) : at<float>(body, 700);
    return game::runtime::ai::entities::pending::direction_turns(x, -at<float>(body, 724));
}

void update_map_tracking(Map *map, float seconds, Byte advance) {
    if (!map->geometry || !map->vertex_count)
        return;
    void *car = game::runtime::physics::local_car(system(68), 0);
    if (!(map->style & 1) || !car) {
        map->translation[0] = map->translation[1] = map->translation[2] = 0;
        for (float &sample : map->heading_history)
            sample = 0;
        for (float &sample : map->zoom_history)
            sample = 0;
        return;
    }
    const auto format = kernel::numeric::current_format();
    const auto E = [&](float value) { return Extended(value, format); };
    void *body = at<void *>(car, 1768);
    const float position[3] = {at<float>(body, 680), at<float>(body, 684),
                               -at<float>(body, 688)};
    auto x = E(position[0]), y = E(position[1]);
    if (map->half_width != 0 && map->half_height != 0) {
        x = (E(map->centre_x) - position[0]) / E(map->half_width);
        y = (E(position[2]) - map->centre_z) / E(map->half_height);
        if (map->reversed)
            x = -x;
    }
    map->translation[1] = float(-y);
    map->translation[0] = float(-x);
    auto heading = map_car_heading(map);
    auto difference = heading - filtered_map_value(map->heading_history);
    if (difference < E(0))
        difference = -difference;
    if (!advance || E(.25f) < difference) {
        map->heading_history[0] = map->heading_history[1] = map->heading_history[2] =
            float(heading);
    } else {
        const float step = float(E(seconds) * .5f);
        const auto filtered = filtered_map_value(map->heading_history);
        if (filtered < heading - step) {
            heading = E(step) + map->heading_history[3];
            map->heading_history[0] = map->heading_history[1];
            map->heading_history[1] = map->heading_history[2];
            map->heading_history[2] = map->heading_history[3];
        } else if (heading + step < filtered) {
            heading = -E(step) + map->heading_history[3];
            map->heading_history[0] = map->heading_history[1];
            map->heading_history[1] = map->heading_history[2];
            map->heading_history[2] = map->heading_history[3];
        } else
            map->heading_history[0] = map->heading_history[1] = map->heading_history[2] =
                float(heading);
    }
    map->heading_history[3] = float(heading);
    auto target = E(0);
    void *entity = at<void *>(car, 7988);
    if (entity && map->pursuit_distance != 0 && map->zoom_rate != 0) {
        void *owner = invoke<void *>(perpetrator_interface(entity), 3);
        void *chase = owner ? at<void *>(owner, 8) : nullptr;
        if (chase) {
            void *nearest = nullptr;
            if (game::runtime::ai::racer(entity))
                nearest = at<void *>(chase, 44);
            else if (invoke<void *>(perpetrator_interface(entity), 2)) {
                float nearest_distance = 0;
                const Dword count = at<Dword>(chase, 40);
                for (Dword index = 0; index < count; ++index) {
                    void *candidate = at<void *>(chase, 28 + index * 4);
                    const auto distance = [&] {
                        return Extended::from_double(
                            double(game::runtime::ai::pair_distance(candidate, entity)), format);
                    };
                    if (!nearest || distance() < E(nearest_distance)) {
                        nearest = candidate;
                        nearest_distance = float(distance());
                    }
                }
            }
            if (nearest) {
                auto ratio =
                    Extended::from_double(double(game::runtime::ai::pair_distance(nearest, entity)),
                                          format) /
                    E(map->pursuit_distance);
                if (!(ratio < E(1)))
                    ratio = E(1);
                target = (E(1) - ratio) * map->pursuit_zoom;
            }
        }
    }
    if (!advance)
        map->zoom_history[0] = map->zoom_history[1] = map->zoom_history[2] = float(target);
    else {
        const bool increasing = filtered_map_value(map->zoom_history) < target;
        auto delta = E(seconds) * map->zoom_rate;
        if (!increasing)
            delta = -delta;
        target = delta + map->zoom_history[3];
        map->zoom_history[0] = map->zoom_history[1];
        map->zoom_history[1] = map->zoom_history[2];
        map->zoom_history[2] = map->zoom_history[3];
    }
    map->zoom_history[3] = float(target);
    const auto filtered = filtered_map_value(map->zoom_history);
    const float stored = float(filtered);
    float result = filtered < E(0) ? 0 : map->pursuit_zoom;
    if (!(filtered < E(0)) && !(map->pursuit_zoom < stored))
        result = stored;
    for (float &sample : map->zoom_history)
        sample = result;
}

void __fastcall prepare_map(Map *map, void *, gui::engine::Engine *engine) {
    gui::widgets::prepare_draw(&map->base.base, nullptr, engine);
    if (map->first_draw) {
        update_map_tracking(map, 0, 0);
        map->first_draw = 0;
    }
}
} // namespace nfs::application::frontend::hud
