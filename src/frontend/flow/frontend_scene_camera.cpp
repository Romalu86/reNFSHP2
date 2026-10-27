#include "frontend_scene_widgets.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/graphics/resources/eagl_objects.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world::camera {

void menu_camera_matrix(void *state, float *matrix) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    float eye[] = {at<float>(state, 4), at<float>(state, 8), float(X(at<float>(state, 12)) * -1.f)};
    float up[] = {at<float>(state, 60), at<float>(state, 64),
                  float(X(at<float>(state, 68)) * -1.f)};
    float target[] = {at<float>(state, 16), at<float>(state, 20), at<float>(state, 24)};
    float backward[] = {float(X(eye[0]) - target[0]), float(X(eye[1]) - target[1]),
                        float(X(eye[2]) - X(target[2]) * -1.f)};
    runtime::normalize_vector(backward, backward);
    float right[3], vertical[3];
    runtime::cross_vectors_entry(up, backward, right);
    runtime::normalize_vector(right, right);
    runtime::cross_vectors_entry(backward, right, vertical);
    matrix[0] = right[0];
    matrix[4] = right[1];
    matrix[8] = right[2];
    matrix[12] = float(-(X(right[1]) * eye[1] + X(right[0]) * eye[0] + X(right[2]) * eye[2]));
    matrix[1] = vertical[0];
    matrix[2] = backward[0];
    matrix[5] = vertical[1];
    matrix[9] = vertical[2];
    matrix[6] = backward[1];
    matrix[13] =
        float(-(X(vertical[1]) * eye[1] + X(vertical[2]) * eye[2] + X(vertical[0]) * eye[0]));
    matrix[10] = backward[2];
    matrix[3] = matrix[7] = matrix[11] = 0;
    matrix[15] = 1;
    matrix[14] =
        float(-(X(backward[1]) * eye[1] + X(backward[0]) * eye[0] + X(backward[2]) * eye[2]));
    if (network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 6) != 2 &&
        network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 6) != 3)
        return;
    if (at<Byte>(subsystem(52), 81296)) {
        void *screen = at<void *>(subsystem(96), 40);
        if (screen && !runtime::invoke<Dword>(screen, 3, "TrackSelectScreen"))
            return;
    }
    for (unsigned i : {0u, 4u, 8u, 12u})
        matrix[i] = float(X(matrix[i]) * -1.f);
}

void apply_menu_camera(void *director, Dword *viewport, float scale, float offset) {
    void *mode =
        find_camera_mode(director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4));
    float matrix[16];
    menu_camera_matrix(bytes(mode) + 4, matrix);
    graphics::viewports::set_matrix(viewport, matrix);
    const auto format = kernel::numeric::current_format();
    graphics::viewports::perspective(
        viewport, at<float>(mode, 32),
        float(kernel::numeric::Extended(at<float>(mode, 52), format) * scale),
        float(kernel::numeric::Extended(at<float>(mode, 56), format) + offset),
        float(kernel::numeric::Extended(at<float>(mode, 60), format) + offset));
}
} // namespace nfs::game::world::camera
namespace nfs::application::frontend {
using kernel::numeric::Extended;

std::uint8_t reset_scene_camera(SceneWidget *widget) {
    widget->zoom = widget->requested_zoom = 0;
    void *camera = game::runtime::invoke<void *>(game::tick_services[0], 2);
    if (camera)
        std::memcpy(game::runtime::bytes(camera) + 4, &widget->camera_position.x, 12);
    assign_vector_value(&widget->initial_camera, &widget->camera_position.x);
    std::memcpy(widget->camera_origin, &widget->initial_camera.x, 12);
    return camera != nullptr;
}

void update_scene_camera(SceneWidget *widget) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    float saved[16], result[16], rotation[16];
    std::memcpy(saved, graphics::viewports::matrix(graphics::main_viewport), sizeof(saved));
    const float x = -widget->initial_position.x, y = float(-X(widget->initial_position.y) - .6f),
                z = -widget->initial_position.z;
    graphics::math::translation(result, x, y, z);
    graphics::math::copy(rotation, saved);
    if (widget->zoom < widget->requested_zoom) {
        auto step = X(widget->requested_zoom) - widget->zoom;
        if (X(.5f) < step)
            step = X(.5f);
        widget->zoom = float(step + widget->zoom);
    } else if (widget->requested_zoom < widget->zoom) {
        auto step = X(widget->zoom) - widget->requested_zoom;
        if (X(.5f) < step)
            step = X(.5f);
        widget->zoom = float(X(widget->zoom) - step);
    }
    float zoom = widget->zoom;
    if (zoom < -1.f)
        zoom = -1.f;
    else if (zoom > 30.f)
        zoom = 30.f;
    widget->zoom = zoom;
    const auto retained_angle = (X(3.1415927410125732421875f) * 0.0055555556900799274444580078125f) * zoom;
    const float angle = float(retained_angle);
    if (retained_angle < X(0) || X(0) < retained_angle || std::isnan(angle)) {
        const auto sine = Extended::from_double(std::sin(double(angle)), format),
                   cosine = Extended::from_double(std::cos(double(angle)), format);
        const auto inverse = X(1) / X(1).square_root();
        const float axis_x = float(inverse), axis_z = float(inverse * 0.f);
        const auto axis_y = inverse * 0.f, complement = X(1) - cosine,
                   product_x = X(axis_x) * complement;
        const float product_y = float(complement * axis_y), product_z = float(complement * axis_z);
        const float sine_x = float(sine * axis_x), sine_y = float(sine * axis_y),
                    sine_z = float(sine * axis_z);
        rotation[0] = float(X(axis_x) * product_x + cosine);
        rotation[1] = float(product_x * axis_y + sine_z);
        rotation[2] = float(product_x * axis_z - sine_y);
        rotation[4] = float(X(product_y) * axis_x - sine_z);
        rotation[5] = float(X(product_y) * axis_y + cosine);
        rotation[6] = float(X(product_y) * axis_z + sine_x);
        rotation[8] = float(X(product_z) * axis_x + sine_y);
        rotation[9] = float(X(product_z) * axis_y - sine_x);
        rotation[10] = float(X(product_z) * axis_z + cosine);
    } else {
        rotation[0] = rotation[5] = rotation[10] = 1;
        rotation[1] = rotation[2] = rotation[4] = rotation[6] = rotation[8] = rotation[9] = 0;
    }
    graphics::math::append(result, rotation);
    graphics::math::append_translation(result, x, y, z);
    graphics::viewports::set_matrix(graphics::main_viewport, result);
}
} // namespace nfs::application::frontend
