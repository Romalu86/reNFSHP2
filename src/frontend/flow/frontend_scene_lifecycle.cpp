#include "frontend_scene_widgets.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using kernel::numeric::Extended;

static SceneLight *construct_scene_light(SceneLight *light) {
    light->colour = {game::runtime::values::vector_methods, 0, 0, 0};
    light->direction = {game::runtime::values::vector_methods, 0, 0, 0};
    light->intensity = 0;
    return light;
}

static void destroy_scene_light(SceneLight *light) {
    light->direction.methods = gui::base_methods;
    light->colour.methods = gui::base_methods;
}

SceneWidget *construct_scene_widget(SceneWidget *widget, void *parent) {
    family::construct_image_widget(widget, parent);
    SceneVector *vectors[] = {
        &widget->position,         &widget->initial_position, &widget->angles,
        &widget->angular_velocity, &widget->angular_step,     &widget->camera_position,
        &widget->initial_camera,   &widget->ambient_sky,      &widget->ambient_ground,
        &widget->sun_position,     &widget->sun_colour};
    for (auto *value : vectors)
        *value = {game::runtime::values::vector_methods, 0, 0, 0};
    widget->pitch = 0;
    widget->rotates = 1;
    widget->drag_flags[0] = widget->drag_flags[1] = 0;
    widget->zoom_step = 0;
    for (auto &light : widget->lights)
        construct_scene_light(&light);
    widget->drag_angles = {game::runtime::values::vector_methods, 0, 0, 0};
    widget->rotation_timer = 0;
    widget->rotation_angle = widget->requested_zoom = widget->zoom = 0;
    widget->base.base.methods = scene_widget_methods.data();
    const float *zero = game::runtime::saved::zero_vector;
    assign_vector_value(&widget->position, zero);
    assign_vector_value(&widget->initial_position, zero);
    assign_vector_value(&widget->angles, zero);
    std::memcpy(widget->camera_origin, zero, sizeof(widget->camera_origin));
    std::memcpy(widget->rotation, game::runtime::saved::identity_matrix, sizeof(widget->rotation));
    assign_vector_value(&widget->camera_position, zero);
    assign_vector_value(&widget->initial_camera, zero);
    std::memcpy(widget->camera_matrix, game::runtime::skids::lifecycle::identity_matrix,
                sizeof(widget->camera_matrix));
    assign_vector_value(&widget->angular_velocity, zero);
    assign_vector_value(&widget->angular_step, zero);
    widget->ambient_sky.x = widget->ambient_sky.y = widget->ambient_sky.z = .3f;
    assign_vector_value(&widget->ambient_ground, zero);
    widget->lights[0].direction.x = .2f;
    widget->lights[0].direction.y = -.8f;
    widget->lights[0].direction.z = 0;
    for (unsigned i = 1; i < 3; ++i) {
        widget->lights[i].direction.x = 0;
        widget->lights[i].direction.y = -.8f;
        widget->lights[i].direction.z = .2f;
    }
    const float sun[] = {0, 9999.990234375f, 0};  
    assign_vector_value(&widget->sun_position, sun);
    widget->drag_y = widget->drag_x = 0;
    assign_vector_value(&widget->drag_angles, zero);
    widget->drag_velocity = 0;
    widget->drag_clock = 0;
    widget->previous_x = 0;
    widget->rotation_timer = gui::create_widget_timer(widget->base.base.id, 0.0333333350718021392822265625f);
    return widget;
}

void destroy_scene_widget(SceneWidget *widget) {
    widget->base.base.methods = scene_widget_methods.data();
    gui::remove_widget_timer(widget->rotation_timer);
    widget->drag_angles.methods = gui::base_methods;
    for (int i = 2; i >= 0; --i)
        destroy_scene_light(&widget->lights[i]);
    SceneVector *vectors[] = {
        &widget->sun_colour,       &widget->sun_position,     &widget->ambient_ground,
        &widget->ambient_sky,      &widget->initial_camera,   &widget->camera_position,
        &widget->angular_step,     &widget->angular_velocity, &widget->angles,
        &widget->initial_position, &widget->position};
    for (auto *value : vectors)
        value->methods = gui::base_methods;
    family::destroy_image_widget(&widget->base.base);
}

void __fastcall tick_scene_widget(SceneWidget *widget, void *, Dword timer) {
    if (timer != widget->rotation_timer)
        return;
    widget->rotation_angle =
        float(Extended(0.0333333350718021392822265625f, kernel::numeric::current_format()) * widget->angular_step.y +
              widget->rotation_angle);
    game::runtime::ai::lifecycle::rotate_y_basis(widget->rotation, widget->angular_step.y,
                                                 widget->rotation);
    rotate_scene_basis_z(widget->rotation, widget->angular_step.z, widget->rotation);
}
// The saved method is RET 4.
void __fastcall draw_scene_widget(SceneWidget *, void *, gui::engine::Engine *) {
}

void __fastcall initialize_scene_widget(SceneWidget *widget, void *) {
    gui::widgets::pending::request_general_layout(&widget->base, nullptr);
    assign_vector_value(&widget->initial_position, &widget->position.x);
}

void __fastcall release_scene_widget(SceneWidget *widget, void *) {
    widget->base.base.options &= ~Dword(2);
}
} // namespace nfs::application::frontend
