#include "ai_path_frame_x87.hpp"
#include "ai_driver_runtime.hpp"
#include "../../core/lifecycle/kernel_values.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

void __fastcall car_recover_path(void *entity, void *, Dword controls_word) {
    int sample;
    Dword node;
    entities::pending::advance_path_distance(bytes(entity) + 220, 0, &sample, &node);
    Dword target[3];
    const auto *point = reinterpret_cast<const Dword *>(node);
    target[0] = point[0];
    target[1] = point[1];
    target[2] = point[2];
    bool no_previous_position;
    {
        values::TaggedValue zero;
        values::reset_value(&zero);
        zero.type = 11;
        zero.payload[0] = 0;
        zero.payload[1] = 0;
        zero.payload[2] = 0;
        values::TaggedValue previous;
        values::reset_value(&previous);
        const Dword x = at<Dword>(entity, 380);
        const Dword y = at<Dword>(entity, 384);
        const Dword z = at<Dword>(entity, 388);
        previous.type = 11;
        previous.payload[0] = x;
        previous.payload[1] = y;
        previous.payload[2] = z;
        no_previous_position = values::equal_values(&previous, &zero);
    }
    auto cache_position = [&]() -> Dword * {
        void *view = entities::interface_at(entity, 3);
        void *body = invoke<void *>(view, 0);
        auto *cached = reinterpret_cast<Dword *>(bytes(view) + 4);
        if (!body) {
            cached[0] = 0;
            cached[1] = 0;
            cached[2] = 0;
        } else {
            cached[0] = at<Dword>(body, 680);
            cached[1] = at<Dword>(body, 684);
            at<float>(cached, 8) = -at<float>(body, 688);
        }
        return cached;
    };
    if (no_previous_position) {
        Dword *cached = cache_position();
        at<Dword>(entity, 380) = cached[0];
        at<Dword>(entity, 384) = cached[1];
        at<Dword>(entity, 388) = cached[2];
    }
    void *controls = reinterpret_cast<void *>(controls_word);
    at<Byte>(controls, 5) = 0;
    at<Byte>(controls, 6) = 255;
    steer_car_to_position(entity, reinterpret_cast<const float *>(target), controls);
    const Dword *cached = cache_position();
    if (lifecycle::pending::path_frame::recovery_path_moved(
            reinterpret_cast<const float *>(bytes(entity) + 380),
            reinterpret_cast<const float *>(cached))) {
        at<Dword>(entity, 380) = 0;
        at<Dword>(entity, 384) = 0;
        at<Dword>(entity, 388) = 0;
        at<Dword>(entity, 404) = 0;
    }
}
} // namespace nfs::game::runtime::ai::cars
