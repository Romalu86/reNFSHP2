#include "../actors/model_render_runtime.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
namespace nfs::game::world::dependency {

void compose_track_object_matrix(void *object, Dword *matrix) {
    auto *output = static_cast<volatile Dword *>(matrix);
    const Dword rotation_xx = at<volatile Dword>(object, 40);
    const Dword rotation_yx = at<volatile Dword>(object, 52);
    output[4] = rotation_yx;
    const Dword rotation_yy = at<volatile Dword>(object, 56);
    output[0] = rotation_xx;
    const Dword rotation_xy = at<volatile Dword>(object, 44);
    output[5] = rotation_yy;
    const Dword rotation_yz = at<volatile Dword>(object, 60);
    output[1] = rotation_xy;
    const Dword rotation_xz = at<volatile Dword>(object, 48);
    output[6] = rotation_yz;
    const Dword rotation_zx = at<volatile Dword>(object, 64);
    output[2] = rotation_xz;
    output[8] = rotation_zx;
    const Dword rotation_zy = at<volatile Dword>(object, 68);
    output[3] = 0;
    output[7] = 0;
    output[11] = 0;
    const Dword position_x = at<volatile Dword>(object, 28);
    output[9] = rotation_zy;
    const Dword rotation_zz = at<volatile Dword>(object, 72);
    output[12] = position_x;
    const Dword position_y = at<volatile Dword>(object, 32);
    const Dword position_z = at<volatile Dword>(object, 36);
    output[10] = rotation_zz;
    output[13] = position_y;
    output[14] = position_z;
    output[15] = 1065353216;
}
namespace {
void draw_object_model(void *object, const Dword *matrix) {
    void *model = at<void *>(object, 88);
    if (!model)
        return;
    void *body = at<void *>(object, 80);
    if (!body || at<Byte>(body, 729))
        vehicle_render::pending::draw_model(model, matrix);
}
} // namespace

void draw_track_object_chain(void *object, Dword view, const float *parent) {
    if (void *sibling = at<void *>(object, 24))
        draw_track_object_chain(sibling, view, parent);
    float local[16], combined[16];
    compose_track_object_matrix(object, reinterpret_cast<Dword *>(local));
    graphics::math::multiply(local, parent, combined);
    draw_object_model(object, reinterpret_cast<Dword *>(combined));
    if (void *child = at<void *>(object, 20))
        draw_track_object_chain(child, view, combined);
}
// Draw the Boom hierarchy.
void pending::draw_boom(void *object, Dword context, void *viewport) {
    void *definition = at<void *>(object, 76);
    auto *sphere = at<float *>(at<void *>(definition, 32), 0);
    if (!graphics::model_render::sphere_visible(static_cast<Dword *>(viewport),
                                                reinterpret_cast<float *>(offset(object, 28)),
                                                sphere[68 / 4]))
        return;
    float local[16];
    compose_track_object_matrix(object, reinterpret_cast<Dword *>(local));
    draw_object_model(object, reinterpret_cast<Dword *>(local));
    if (void *child = at<void *>(object, 20)) {
        if (void *sibling = at<void *>(child, 24))
            draw_track_object_chain(sibling, context, local);
        float child_matrix[16], combined[16];
        compose_track_object_matrix(child, reinterpret_cast<Dword *>(child_matrix));
        graphics::math::multiply(child_matrix, local, combined);
        draw_object_model(child, reinterpret_cast<Dword *>(combined));
        if (void *nested = at<void *>(child, 20))
            draw_track_object_chain(nested, context, combined);
    }
}
} // namespace nfs::game::world::dependency
