#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_scene_widgets.hpp"
#include "../../gui/lifecycle/gui_configuration_values.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../ai/driving/ai_route_planner.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
namespace nfs::application::frontend {
using game::runtime::invoke;
SceneVector scene_default_vector{};

static SceneVector *construct_scene_default(SceneVector *value) {
    value->methods = game::runtime::values::vector_methods;
    value->x = value->y = value->z = 0;
    return value;
}

static void destroy_scene_default() {
    game::runtime::values::destroy_vector(&scene_default_vector);
}

static void initialize_scene_default() {
    construct_scene_default(&scene_default_vector);
    std::atexit(destroy_scene_default);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_scene_default, ".CRT$XCU_NFS_006906C4",
                       initialize_scene_default);
} // namespace

Dword read_scene_vector(void *configuration, const char *key, SceneVector *value,
                       const SceneVector *fallback) {
    const auto *text = invoke<const gui::String *>(configuration, 5, key);
    if (text->length)
        return Dword(std::sscanf(gui::string_ascii(text), "[ %f , %f, %f ]", &value->x, &value->y,
                                &value->z));
    value->x = fallback->x;
    value->y = fallback->y;
    value->z = fallback->z;
    return 0;
}

Dword write_scene_vector(void *configuration, const char *key, const SceneVector *value, int digits,
                        const SceneVector *fallback) {
    auto equal = [](float a, float b) { return a == b; };
    const Dword is_default = equal(value->x, fallback->x) && equal(value->y, fallback->y) &&
                            equal(value->z, fallback->z);
    char format[256], buffer[256]{};
    std::sprintf(format, "[%s%d%s, %s%d%s, %s%d%s]", "%0.", digits, "f", "%0.", digits, "f", "%0.",
                 digits, "f");
    std::sprintf(buffer, format, double(value->x), double(value->y), double(value->z));
    pending::LocalFrontendString text(buffer);
    return invoke<Dword>(configuration, 7, key, &text.value, "VECTOR", Dword(0), is_default);
}

Dword __fastcall load_scene_widget(SceneWidget *widget, void *, void *configuration) {
    using namespace gui::engine::pending;
    read_scene_vector(configuration, "EAGLWidget.mPosition", &widget->position,
                      &scene_default_vector);
    read_scene_vector(configuration, "EAGLWidget.mAngularVelocity", &widget->angular_velocity,
                      &scene_default_vector);
    game::runtime::scale_vectors(1, &widget->angular_velocity.x, 0.00277777784503996372222900390625f,
                                 &widget->angular_step.x);
    SceneVector rotation{game::runtime::values::vector_methods, 0, 0, 0};
    read_scene_vector(configuration, "EAGLWidget.mRotation", &rotation, &scene_default_vector);
    int rotates = 0;
    read_int(configuration, "EAGLWidget.mRotates", &rotates, 0);
    widget->rotates = rotates != 0;
    const float heading =
        float(game::runtime::ai::entities::pending::direction_turns(rotation.x, rotation.z));
    scene_euler_matrix(0, heading, 0, widget->rotation);
    read_float(configuration, "EAGLWidget.mPitch", &widget->pitch, 0);
    game::runtime::matrices::rotate_x(
        widget->rotation,
        float(kernel::numeric::Extended(widget->pitch, kernel::numeric::current_format()) *
              0.00277777784503996372222900390625f));
    read_float(configuration, "EAGLWidget.mZoomStep", &widget->zoom_step, 0);
    read_scene_vector(configuration, "EAGLWidget.mAmbientSky", &widget->ambient_sky,
                      &scene_default_vector);
    read_scene_vector(configuration, "EAGLWidget.mAmbientGround", &widget->ambient_ground,
                      &scene_default_vector);
    read_scene_vector(configuration, "EAGLWidget.mSunPosition", &widget->sun_position,
                      &scene_default_vector);
    const char *colour_keys[] = {"EAGLWidget.mFElights[0].mColor", "EAGLWidget.mFElights[1].mColor",
                                 "EAGLWidget.mFElights[2].mColor"};
    const char *direction_keys[] = {"EAGLWidget.mFElights[0].mDirection",
                                    "EAGLWidget.mFElights[1].mDirection",
                                    "EAGLWidget.mFElights[2].mDirection"};
    const char *intensity_keys[] = {"EAGLWidget.mFElights[0].mIntensity",
                                    "EAGLWidget.mFElights[1].mIntensity",
                                    "EAGLWidget.mFElights[2].mIntensity"};
    for (unsigned i = 0; i < 3; ++i)
        read_scene_vector(configuration, colour_keys[i], &widget->lights[i].colour,
                          &scene_default_vector);
    for (unsigned i = 0; i < 3; ++i)
        read_scene_vector(configuration, direction_keys[i], &widget->lights[i].direction,
                          &scene_default_vector);
    for (unsigned i = 0; i < 3; ++i)
        read_float(configuration, intensity_keys[i], &widget->lights[i].intensity, 0);
    int mirror = 0;
    read_int(configuration, "EAGLWidget.mMirrorImage", &mirror, 0);
    widget->mirrored = mirror != 0;
    return reinterpret_cast<Dword(__thiscall *)(void *, void *)>(
        gui::widgets::family::image_widget_methods[4])(widget, configuration);
}

Dword __fastcall save_scene_widget(SceneWidget *widget, void *, void *configuration) {
    using namespace gui::engine::pending;
    write_scene_vector(configuration, "EAGLWidget.mPosition", &widget->position, 2,
                       &scene_default_vector);
    SceneVector rotation{game::runtime::values::vector_methods, 0, 0, 0};
    scene_euler_angles(widget->rotation, &rotation.x, &rotation.y, &rotation.z);
    rotation.x = float(game::runtime::turn_sine(rotation.y));
    rotation.z = -float(game::runtime::turn_cosine(rotation.y));
    rotation.y = 0;
    write_scene_vector(configuration, "EAGLWidget.mRotation", &rotation, 2, &scene_default_vector);
    write_float(configuration, "EAGLWidget.mPitch", widget->pitch, 2, 0);
    write_int(configuration, "EAGLWidget.mRotates", widget->rotates, 0);
    write_scene_vector(configuration, "EAGLWidget.mAngularVelocity", &widget->angular_velocity, 2,
                       &scene_default_vector);
    write_float(configuration, "EAGLWidget.mZoomStep", widget->zoom_step, 2, 0);
    write_scene_vector(configuration, "EAGLWidget.mAmbientSky", &widget->ambient_sky, 2,
                       &scene_default_vector);
    write_scene_vector(configuration, "EAGLWidget.mAmbientGround", &widget->ambient_ground, 2,
                       &scene_default_vector);
    write_scene_vector(configuration, "EAGLWidget.mSunPosition", &widget->sun_position, 2,
                       &scene_default_vector);
    const char *colour_keys[] = {"EAGLWidget.mFElights[0].mColor", "EAGLWidget.mFElights[1].mColor",
                                 "EAGLWidget.mFElights[2].mColor"};
    const char *direction_keys[] = {"EAGLWidget.mFElights[0].mDirection",
                                    "EAGLWidget.mFElights[1].mDirection",
                                    "EAGLWidget.mFElights[2].mDirection"};
    const char *intensity_keys[] = {"EAGLWidget.mFElights[0].mIntensity",
                                    "EAGLWidget.mFElights[1].mIntensity",
                                    "EAGLWidget.mFElights[2].mIntensity"};
    for (unsigned i = 0; i < 3; ++i)
        write_scene_vector(configuration, colour_keys[i], &widget->lights[i].colour, 2,
                           &scene_default_vector);
    for (unsigned i = 0; i < 3; ++i)
        write_scene_vector(configuration, direction_keys[i], &widget->lights[i].direction, 2,
                           &scene_default_vector);
    for (unsigned i = 0; i < 3; ++i)
        write_float(configuration, intensity_keys[i], widget->lights[i].intensity, 2, 0);
    write_int(configuration, "EAGLWidget.mMirrorImage", widget->mirrored, 0);
    return reinterpret_cast<Dword(__thiscall *)(void *, void *)>(
        gui::widgets::family::image_widget_methods[5])(widget, configuration);
}
} // namespace nfs::application::frontend
