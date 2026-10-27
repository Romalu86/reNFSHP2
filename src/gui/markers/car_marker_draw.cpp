#include "car_marker_runtime.hpp"
#include "../lifecycle/gui_images.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {

void draw_vehicle_marker(void *entity, void *view, Dword red, Dword green, Dword blue) {
    if (gui::engine::images::suspend_drawing())
        return;
    auto *camera = invoke<Dword *>(view, 2);
    if (!camera)
        return;
    float position[3];
    std::memcpy(position, invoke<const float *>(entity, 0), 12);
    const Dword colour = (((red & 255) | 4294942464u) << 8 | (green & 255)) << 8 | (blue & 255);
    const Dword outline = 4278190080u;
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
    gp::alpha_blend_mode(state.fields, 2);
    gp::transparency(state.fields, 1);
    gp::culling(state.fields, 0);
    gp::depth_method(state.fields, 2);
    gp::transparency(state.fields, 1);
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
    const auto projection = runtime::retained_dot_product(delta, orientation + 6);
    const float depth = float(projection);
    // TEST AH,41 / JE and TEST AH,5 / JNP reject ordered out-of-range values.
    if (E(-2) < projection || depth < -240.f)
        return;
    graphics::backend::set_depth_write(graphics::context, 0);
    auto *draw = graphics::bindings::single_draw;
    graphics::immediate::flush(draw);
    std::memcpy(bytes(draw) + 84, matrix, sizeof(matrix));
    graphics::immediate::set_gouraud_state(draw, state.fields);
    graphics::immediate::set_primitive(draw, 4);
    const float absolute_depth = depth < 0 ? -depth : depth;
    const auto radius =
        Extended::from_double(std::pow(double(absolute_depth), double(0.1f)), format) * 0.5f;
    const auto height =
        radius + at<float>(at<void *>(at<void *>(entity, 368), 1768), 124) + 0.2f + position[1];
    // The saved stack retains right X/Y and up X, but stores the other products as float.
    const auto right_x = E(orientation[0]) * radius;
    const auto right_y = E(orientation[1]) * radius;
    const float right_z = float(E(orientation[2]) * radius);
    const auto up_x = E(orientation[3]) * radius;
    const float up_y = float(E(orientation[4]) * radius);
    const float up_z = float(E(orientation[5]) * radius);
    float vertices[3][4];
    vertices[0][0] = float(-right_x + position[0]);
    vertices[0][1] = float(-right_y + height);
    vertices[0][2] = float(-E(right_z) + position[2]);
    vertices[0][3] = 1.f;
    vertices[1][0] = float(E(position[0]) + right_x);
    vertices[1][1] = float(right_y + height);
    vertices[1][2] = float(E(position[2]) + right_z);
    vertices[1][3] = 1.f;
    vertices[2][0] = float(-up_x + position[0]);
    vertices[2][1] = float(-E(up_y) + height);
    vertices[2][2] = float(-E(up_z) + position[2]);
    vertices[2][3] = 1.f;
    for (unsigned vertex = 0; vertex < 3; ++vertex)
        graphics::immediate::vertex(draw, vertices[vertex], &colour);
    graphics::immediate::end_primitive(draw);
    graphics::immediate::flush(draw);
    gp::primitive(state.fields, 2);
    gp::depth_method(state.fields, 4);
    gp::alpha_blend_mode(state.fields, 1);
    graphics::immediate::set_gouraud_state(draw, state.fields);
    graphics::immediate::set_primitive(draw, 2);
    for (unsigned vertex = 0; vertex < 3; ++vertex) {
        graphics::immediate::vertex(draw, vertices[vertex], &outline);
        graphics::immediate::vertex(draw, vertices[(vertex + 1) % 3], &outline);
    }
    graphics::immediate::end_primitive(draw);
    graphics::backend::set_depth_write(graphics::context, 1);
}
} // namespace nfs::game::runtime::ai::cars
