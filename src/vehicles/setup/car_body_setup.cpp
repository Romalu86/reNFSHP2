#include "car_initialization_runtime.hpp"
namespace nfs::game::world::vehicle_setup {

void adjust_body_vertices(Dword shape, void *vertices) {
    const auto format = kernel::numeric::current_format();
    auto scale = [&](unsigned field, float factor) {
        at<float>(vertices, field) = float(Extended(at<float>(vertices, field), format) * factor);
    };
    switch (shape) {
    case 0:
    case 3:
        return;
    case 1:
        for (unsigned field : {88u, 76u, 208u})
            scale(field, .3f);
        for (unsigned field : {52u, 64u, 172u})
            scale(field, .5f);
        for (unsigned field : {8u, 20u, 224u, 56u, 68u, 176u})
            scale(field, .9f);
        return;
    case 2:
        for (unsigned field : {80u, 212u, 92u})
            scale(field, .8f);
        scale(116, .95f);
        break;
    case 4:
        break;
    case 6:
        for (unsigned field : {184u, 180u, 196u, 192u})
            scale(field, .8f);
        scale(60, .6f);
        scale(48, .6f);
        scale(64, .3f);
        scale(52, .3f);
        scale(172, .4f);
        scale(68, .72f);
        scale(56, .75f);
        scale(176, .72f);
        scale(72, .6f);
        scale(84, .6f);
        scale(44, .8f);
        scale(32, .8f);
        scale(260, .9f);
        scale(88, .5f);
        scale(76, .5f);
        scale(208, .7f);
        scale(264, .8f);
        scale(272, .9f);
        scale(276, .8f);
        for (unsigned field : {284u, 288u, 296u, 300u, 308u})
            scale(field, .9f);
        return;
    case 7:
        for (unsigned field : {52u, 64u, 172u})
            scale(field, .3f);
        return;
    default:
        for (unsigned field : {88u, 76u, 208u})
            scale(field, .3f);
        scale(52, .4f);
        scale(64, .4f);
        scale(172, .4f);
        return;
    }
    scale(68, .8f);
    scale(176, .9f);
    scale(56, .8f);
    scale(88, .3f);
    scale(76, .3f);
    scale(208, .4f);
    scale(52, .3f);
    scale(64, .3f);
    scale(172, .4f);
}

void configure_body_shape(void *car) {
    void *body = at<void *>(car, 1768);
    const Dword shape = at<Dword>(car, 552);
    adjust_body_vertices(shape, bytes(body) + 140);
    const bool narrow_x = shape != 0 && shape != 6 && shape != 7;
    const bool narrow_z = shape != 0 && shape != 1 && shape != 7;
    at<float>(body, 468) = narrow_x ? .8f : .9f;
    at<float>(body, 472) = 1;
    at<float>(body, 476) = narrow_z ? .8f : .95f;
    at<Dword>(body, 464) = at<Dword>(car, 500);
    const auto format = kernel::numeric::current_format();
    at<float>(body, 52) = float(Extended(at<float>(car, 104), format) - at<float>(car, 112));
    at<float>(body, 48) =
        float(Extended(at<float>(car, 544), format) * 0.90909087657928466796875f - at<float>(body, 124));
}

void adjust_body_for_tires(void *car) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto front_difference = E(at<float>(car, 352)) - at<float>(car, 356);
    if (E(0) < front_difference)
        front_difference = E(0);
    auto rear_difference = E(at<float>(car, 356)) - at<float>(car, 352);
    if (E(0) < rear_difference)
        rear_difference = E(0);
    void *body = at<void *>(car, 1768);
    for (unsigned field : {144u, 156u})
        at<float>(body, field) = float(E(at<float>(body, field)) - front_difference);
    for (unsigned field : {168u, 180u})
        at<float>(body, field) = float(E(at<float>(body, field)) - rear_difference);
    at<float>(body, 396) = float(E(at<float>(car, 356)) + at<float>(body, 396));
    const auto radius = (E(at<float>(car, 356)) + at<float>(car, 352)) * .5f;
    for (unsigned field : {372u, 300u, 384u})
        at<float>(body, field) = float(radius + at<float>(body, field));
    const auto front = E(at<float>(car, 104)) + at<float>(car, 352);
    at<float>(body, 148) = at<float>(body, 160) = float(front);
    const auto rear = E(at<float>(car, 108)) - at<float>(car, 356);
    at<float>(body, 172) = at<float>(body, 184) = at<float>(body, 400) = float(rear);
}

void mark_bounds_dirty(void *renderable) {
    at<Byte>(renderable, 3968) = 1;
}

void query_render_bounds(void *renderable, float *minimum, float *maximum) {
    const float max_x = at<float>(renderable, 3992), min_x = at<float>(renderable, 3980);
    minimum[0] = min_x;
    const Dword min_z = at<Dword>(renderable, 3988);
    at<Dword>(minimum, 4) = at<Dword>(renderable, 3984);
    at<Dword>(minimum, 8) = min_z;
    const float max_y = at<float>(renderable, 3996);
    maximum[0] = max_x;
    const float min_y = at<float>(renderable, 3984);
    const Dword max_z = at<Dword>(renderable, 4000);
    maximum[1] = max_y;
    at<Dword>(maximum, 8) = max_z;
    const auto format = kernel::numeric::current_format();
    at<float>(renderable, 4028) = float((Extended(max_x, format) - min_x) * .5f);
    at<float>(renderable, 4032) = float((Extended(max_y, format) - min_y) * .5f);
    at<float>(renderable, 4036) = float(
        (Extended(at<float>(renderable, 4000), format) - at<float>(renderable, 3988)) * .5f);
}
} // namespace nfs::game::world::vehicle_setup
