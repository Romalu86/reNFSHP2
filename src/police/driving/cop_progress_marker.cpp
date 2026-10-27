#include "../../gui/markers/car_marker_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {

void draw_cop_progress_marker(void *entity, void *view) {
    auto *camera = invoke<Dword *>(view, 2);
    if (!camera)
        return;
    float position[3];
    std::memcpy(position, invoke<const float *>(entity, 0), 12);
    const Dword colour = 4294901760u, black = 4278190080u;
    struct RenderState {
        Dword fields[17];
        RenderState() {
            ::nfs::lifecycle::construction::render_state(fields);
        }
        ~RenderState() {
            ::nfs::lifecycle::construction::destruction::render_state_member(fields);
        }
    } state;
    namespace gp = graphics::geoprim;
    gp::primitive(state.fields, 4);
    gp::texture_enable(state.fields, 0);
    gp::shading(state.fields, 1);
    gp::alpha_test(state.fields, 0);
    gp::alpha_blend_mode(state.fields, 0);
    gp::transparency(state.fields, 1);
    gp::culling(state.fields, 0);
    gp::depth_method(state.fields, 2);
    Dword matrix[16];
    std::memcpy(matrix, skids::lifecycle::identity_matrix, sizeof(matrix));
    float camera_position[3], orientation[9];
    std::memcpy(camera_position, bytes(camera) + 4, 12);
    race_audio::camera_orientation(camera, orientation);
    orientation[2] = -orientation[2];
    orientation[6] = -orientation[6];
    orientation[7] = -orientation[7];
    orientation[8] = -orientation[8];
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    float delta[3] = {float(E(position[0]) - camera_position[0]),
                      float(E(position[1]) - camera_position[1]),
                      float(E(position[2]) + camera_position[2])};
    auto projection = runtime::retained_dot_product(delta, orientation + 6);
    // The clipping branches let unordered depth continue to the draw path.
    if (E(-2) < projection || projection < E(-240))
        return;
    if (projection < E(0))
        projection = -projection;
    const float depth = float(projection);
    const auto radius = Extended::from_double(std::pow(double(depth), double(0.1f)), format) * 0.5f;
    const float half_height = float(E(0.2f) * radius);
    position[1] =
        float(E(0.8f) * radius + at<float>(at<void *>(at<void *>(entity, 368), 1768), 124) +
              0.2f + position[1]);
    float right[3], up[3];
    runtime::scale_vectors(1, orientation, float(-radius), right);
    runtime::scale_vectors(1, orientation + 3, half_height, up);
    const auto scaled_progress = E(at<float>(entity, 660)) * 0.95f;
    const auto progress = scaled_progress + scaled_progress - E(1);
    float vertices[6][4];
    for (unsigned axis = 0; axis < 3; ++axis) {
        vertices[0][axis] = float(E(up[axis]) + right[axis] + position[axis]);
        const auto moved = E(right[axis]) * progress;
        const float stored_moved = float(moved);
        vertices[1][axis] = float(E(up[axis]) + moved + position[axis]);
        vertices[2][axis] = float(E(up[axis]) + (E(position[axis]) - right[axis]));
        vertices[3][axis] = float((E(position[axis]) - right[axis]) - up[axis]);
        const auto retained_moved = axis ? E(stored_moved) : moved;
        vertices[4][axis] = float((retained_moved + position[axis]) - up[axis]);
        vertices[5][axis] = float((E(right[axis]) + position[axis]) - up[axis]);
    }
    for (unsigned vertex = 0; vertex < 6; ++vertex)
        vertices[vertex][3] = 1.f;
    graphics::backend::set_depth_write(graphics::context, 0);
    graphics::immediate::flush(graphics::bindings::single_draw);
    auto *draw = graphics::bindings::single_draw;
    std::memcpy(bytes(draw) + 84, matrix, sizeof(matrix));
    graphics::immediate::set_gouraud_state(draw, state.fields);
    graphics::immediate::set_primitive(graphics::bindings::single_draw, 4);
    for (unsigned vertex : {0u, 1u, 4u, 0u, 4u, 5u})
        graphics::immediate::vertex(graphics::bindings::single_draw, vertices[vertex], &colour);
    for (unsigned vertex : {1u, 2u, 3u, 4u, 1u, 3u})
        graphics::immediate::vertex(graphics::bindings::single_draw, vertices[vertex], &black);
    graphics::immediate::end_primitive(graphics::bindings::single_draw);
    graphics::immediate::flush(graphics::bindings::single_draw);
    gp::primitive(state.fields, 2);
    gp::depth_method(state.fields, 4);
    gp::alpha_blend_mode(state.fields, 1);
    graphics::immediate::set_gouraud_state(graphics::bindings::single_draw, state.fields);
    graphics::immediate::set_primitive(graphics::bindings::single_draw, 2);
    for (unsigned vertex : {0u, 2u, 2u, 3u, 3u, 5u, 5u, 0u})
        graphics::immediate::vertex(graphics::bindings::single_draw, vertices[vertex], &black);
    graphics::immediate::end_primitive(graphics::bindings::single_draw);
    graphics::backend::set_depth_write(graphics::context, 1);
}
} // namespace nfs::game::runtime::ai::cars
