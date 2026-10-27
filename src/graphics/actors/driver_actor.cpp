#include "../animation/animation_resource_runtime.hpp"
#include "model_render_runtime.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/graphics/display/graphics_error.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <initializer_list>
namespace nfs::game::world::actor_resources {

void *construct_actor(void *actor) {
    using namespace vehicle_render;
    at<volatile Byte>(actor, 41) = 0;
    at<volatile Byte>(actor, 40) = 0;
    at<volatile Dword>(actor, 36) = 0;
    at<volatile Dword>(actor, 44) = 0;
    at<volatile Dword>(actor, 48) = 0;
    at<const Dword *>(actor, 0) = world::pending::boom_base_methods;
    at<volatile Dword>(actor, 8) = 0;
    at<volatile Dword>(actor, 12) = 0;
    construct_actor_channels(offset(actor, 52));
    try {
        construct_node(offset(actor, 16));
    } catch (...) {
        destroy_actor_channels(offset(actor, 52));
        throw;
    }
    const auto clear_vector = [actor](unsigned start) {
        for (unsigned axis = 0; axis < 3; ++axis)
            at<volatile Dword>(actor, start + axis * 4) = 0;
    };
    for (unsigned start : {104u, 124u, 136u, 148u})
        clear_vector(start);
    at<volatile Byte>(actor, 84) = 0;
    at<volatile Dword>(actor, 88) = 0;
    at<volatile Dword>(actor, 92) = 0;
    at<volatile Dword>(actor, 96) = Dword(-1);
    at<volatile Dword>(actor, 100) = 0;
    at<volatile Dword>(actor, 116) = 0;
    at<volatile Dword>(actor, 120) = 1065353216;
    for (unsigned start : {160u, 180u, 192u, 244u})
        clear_vector(start);
    at<volatile Dword>(actor, 232) = 0;
    at<volatile Dword>(actor, 204) = 0;
    at<volatile Dword>(actor, 236) = 0;
    at<volatile Dword>(actor, 240) = 0;
    at<volatile std::uint16_t>(actor, 256) = 0;
    at<volatile std::uint16_t>(actor, 258) = 0;
    at<volatile std::uint16_t>(actor, 260) = 0;
    clear_vector(220);
    clear_vector(208);
    at<Dword>(actor, 264) = 0;
    for (unsigned field = 384; field <= 412; field += 4)
        at<Dword>(actor, field) = 0;
    std::memcpy(offset(actor, 272), runtime::skids::lifecycle::identity_matrix, 64);
    at<volatile Dword>(actor, 144) = 1065353216;
    at<volatile Dword>(actor, 156) = 1065353216;
    at<volatile Dword>(actor, 164) = 1065353216;
    at<volatile Byte>(actor, 372) = 0;
    at<volatile Byte>(actor, 373) = 0;
    at<Dword>(actor, 376) = 1051372191;
    at<const Dword *>(actor, 0) = actor_methods;
    at<const Dword *>(actor, 16) = actor_reference_methods;
    std::strcpy(reinterpret_cast<char *>(offset(actor, 416)), "actor");
    return actor;
}

void destroy_actor(void *actor) {
    at<const Dword *>(actor, 0) = actor_methods;
    at<const Dword *>(actor, 16) = actor_reference_methods;
    void *table = at<void *>(subsystem(48), 108);
    if (void *texture = at<void *>(actor, 396)) {
        runtime::events::release(static_cast<Dword *>(texture));
        texture = at<void *>(actor, 396);
        if (!at<Dword>(texture, 4)) {
            world::unlink_list_node(offset(at<void *>(actor, 392), 160), texture);
            if (texture)
                world::invoke(texture, 0, 1u);
        }
        at<Dword>(actor, 396) = 0;
    }
    if (void *model = at<void *>(actor, 392)) {
        runtime::events::release(reinterpret_cast<Dword *>(offset(model, 180)));
        model = at<void *>(actor, 392);
        if (!at<Dword>(model, 184)) {
            world::unlink_list_node(at<void *>(actor, 388),
                                    model ? offset(model, 180) : nullptr);
            if (model)
                world::invoke(model, 0, 1u);
        }
        at<Dword>(actor, 392) = 0;
    }
    at<Dword>(actor, 384) = 0;
    if (void *character = at<void *>(actor, 388)) {
        runtime::events::release(reinterpret_cast<Dword *>(offset(character, 20)));
        character = at<void *>(actor, 388);
        if (!at<Dword>(character, 24)) {
            world::unlink_list_node(table, character ? offset(character, 20) : nullptr);
            if (character)
                world::invoke(character, 0, 1u);
        }
        at<Dword>(actor, 388) = 0;
    }
    destroy_node(offset(actor, 16));
    vehicle_render::destroy_actor_channels(offset(actor, 52));
    at<const Dword *>(actor, 0) = world::pending::boom_base_methods;
}

void *__fastcall delete_actor(void *actor, void *, Dword flags) {
    destroy_actor(actor);
    if (flags & 1)
        kernel::memory_free(actor);
    return actor;
}

void *__fastcall delete_actor_reference(void *reference, void *, Dword flags) {
    return delete_actor(static_cast<Byte *>(reference) - 16, nullptr, flags);
}

const char *__fastcall actor_name(void *reference, void *) {
    return reinterpret_cast<const char *>(offset(reference, 400));
}

void actor_displacement(void *actor, float *output) {
    float orientation[16] = {at<float>(actor, 144),  0, at<float>(actor, 136), 0, 0, 1, 0, 0,
                             -at<float>(actor, 136), 0, at<float>(actor, 144), 0, 0, 0, 0, 1};
    graphics::math::transform_position(reinterpret_cast<float *>(offset(actor, 104)), orientation,
                                       output);
}

void __fastcall actor_position(void *actor, void *, float *output) {
    if (!(at<Byte>(actor, 268) & 3)) {
        float displacement[3];
        actor_displacement(actor, displacement);
        for (unsigned axis = 0; axis < 3; ++axis) {
            const volatile float position = displacement[axis] + at<float>(actor, 124 + axis * 4);
            output[axis] = position;
        }
    } else {
        for (unsigned axis = 0; axis < 3; ++axis)
            at<volatile Dword>(output, axis * 4) = at<volatile Dword>(actor, 124 + axis * 4);
    }
    output[2] = -output[2];
}
// Complete return-zero leaf.
Dword __fastcall actor_body(void *, void *) {
    return 0;
}

void __fastcall actor_rotation(void *, void *, Dword *output) {
    std::memcpy(output, runtime::saved::identity_matrix, 36);
}
const Dword actor_methods[6] = {reinterpret_cast<Dword>(actor_position),
                               reinterpret_cast<Dword>(delete_actor),
                               reinterpret_cast<Dword>(pending::draw_actor),
                               reinterpret_cast<Dword>(pending::advance_actor),
                               reinterpret_cast<Dword>(actor_body),
                               reinterpret_cast<Dword>(actor_rotation)};
const Dword actor_reference_methods[2] = {reinterpret_cast<Dword>(delete_actor_reference),
                                         reinterpret_cast<Dword>(actor_name)};

void *create_actor(void *system, const char *label) {
    void *actor = lifecycle::allocate(464, label, lifecycle::character_pool);
    if (actor) {
        try {
            construct_actor(actor);
        } catch (...) {
            kernel::memory_free(actor);
            throw;
        }
    }
    if (void *list = at<void *>(system, 92)) {
        runtime::events::acquire(reinterpret_cast<Dword *>(offset(actor, 16)));
        world::invoke(list, 1, actor ? offset(actor, 16) : nullptr);
    }
    return actor;
}
// The first stack word is ignored, RET12.
void *acquire_actor_model(void *, void *character, const char *resource, const char *model) {
    return acquire_character_model(character, resource, model);
}
// The first stack word is ignored, RET16.
void *acquire_actor_texture(void *, void *model, const char *resource, const char *variant,
                            const char *texture) {
    return acquire_character_texture(offset(model, 160), resource, variant, texture);
}

void bind_actor_texture(void *model, void *texture) {
    Dword *image = at<Dword *>(texture, 20);
    if (at<Dword *>(model, 148) != image) {
        vehicle_render::bind_model_fsh(at<void *>(model, 128), image);
        at<Dword *>(model, 148) = image;
    }
}

void bind_actor_resources(void *actor, const char *resource, const char *model_name,
                          const char *texture_name, Byte animations,
                          ActorAnimationCallback animation_callback,
                          ActorFrameCallback frame_callback) {
    void *table = at<void *>(subsystem(48), 108);
    void *character = acquire_character(table, resource);
    at<void *>(actor, 388) = character;
    runtime::events::acquire(reinterpret_cast<Dword *>(offset(character, 20)));
    void *model = acquire_actor_model(table, character, resource, model_name);
    at<void *>(actor, 392) = model;
    runtime::events::acquire(reinterpret_cast<Dword *>(offset(model, 180)));
    void *texture = acquire_actor_texture(table, model, resource, model_name, texture_name);
    at<void *>(actor, 396) = texture;
    runtime::events::acquire(static_cast<Dword *>(texture));
    at<void *>(actor, 88) = model;
    bind_actor_texture(model, texture);
    if (animations)
        at<void *>(actor, 384) = offset(character, 40);
    at<ActorAnimationCallback>(actor, 400) = animation_callback;
    at<ActorFrameCallback>(actor, 404) = frame_callback;
}

void set_actor_identifier(void *actor, Dword identifier) {
    at<Dword>(actor, 172) = identifier;
    std::sprintf(reinterpret_cast<char *>(offset(actor, 416)), "actor%d", int(identifier));
}

void reset_animation_clock(void *owner) {
    const Dword now = graphics::unresolved::camera_state;
    at<volatile Byte>(owner, 8) = 0;
    at<volatile Dword>(owner, 0) = now;
}

float animation_duration(void *actor, Dword identifier) {
    if (!identifier)
        return 0;
    void *table = at<void *>(actor, 384);
    void *rotation = at<void *>(vehicle_render::find_animation_entry(table, identifier), 24);
    void *entry = vehicle_render::find_animation_entry(table, identifier + 1000000);
    void *translation = entry ? at<void *>(entry, 24) : nullptr;
    vehicle_render::find_animation_entry(table, identifier + 2000000);
    if (!translation)
        translation = rotation;
    float duration;
    world::invoke(translation, 4, &duration);
    return duration;
}

void yaw_actor(void *actor, float degrees) {
    if (void *child = at<void *>(actor, 264))
        yaw_actor(child, degrees);
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return kernel::numeric::Extended(f, format); };
    const auto retained_radians = X(3.1415927410125732421875f) * 0.0055555556900799274444580078125f * degrees;
    float radians = float(retained_radians), rotation[16];

    // only ordered zero takes the identity branch (TEST AH,44h / JP).
    if (!std::isnan(double(retained_radians)) &&
        !(retained_radians < X(0)) && !(X(0) < retained_radians))
        graphics::math::diagonal(rotation, 1, 1, 1, 1);
    else {
        auto sine = kernel::numeric::Extended::from_double(std::sin(double(radians)), format);
        auto cosine = kernel::numeric::Extended::from_double(std::cos(double(radians)), format);
        auto zero = X(0) * X(1), complement = X(1) - cosine;
        float z = float(zero), y = 1;
        auto product_x = X(z) * complement;
        float product_y = float(complement * y), product_z = float(complement * zero);
        float sine_x = float(sine * z), sine_y = float(sine * y), sine_z = float(sine * zero);
        rotation[0] = float(X(z) * product_x + cosine);
        rotation[1] = float(X(y) * product_x + sine_z);
        rotation[2] = float(product_x * zero - sine_y);
        rotation[3] = 0;
        rotation[4] = float(X(product_y) * z - sine_z);
        rotation[5] = float(X(product_y) * y + cosine);
        rotation[6] = float(X(product_y) * zero + sine_x);
        rotation[7] = 0;
        rotation[8] = float(X(product_z) * z + sine_y);
        rotation[9] = float(X(product_z) * y - sine_x);
        rotation[10] = float(X(product_z) * zero + cosine);
        rotation[11] = rotation[12] = rotation[13] = rotation[14] = 0;
        rotation[15] = 1;
    }
    float direction[3];
    std::memcpy(direction, offset(actor, 136), 12);
    graphics::math::transform_position(direction, rotation, direction);
    runtime::normalize_vector(direction, direction);
    std::memcpy(offset(actor, 136), direction, 12);
}
} // namespace nfs::game::world::actor_resources
namespace nfs::game::world::vehicle_render::pending {

void initialize_driver_actor(void *owner) {
    using namespace actor_resources;
    void *actor = create_actor(subsystem(48), "VehicleDriver - InitActor");
    at<void *>(owner, 52) = actor;
    allocate_animation_channels(offset(actor, 52), "VehiclDriver animation Channel", 3,
                                lifecycle::character_pool);
    char texture[32];
    std::sprintf(texture, "Tex%d.fsh", 1);
    if (!at<Dword>(owner, 56))
        bind_actor_resources(actor, "Driver", "Model1", texture, 1, 0, 0);
    else if (at<Dword>(owner, 56) == 1)
        bind_actor_resources(actor, "CopDriver", "Model1", texture, 1, 0, 0);
    select_character_animation(actor, 90902, 0);
    at<Dword>(actor, 376) = 1051372191;
    at<Byte>(actor, 61) = 0;
    yaw_actor(actor, 0);
    at<Dword>(actor, 176) = 1148846080;
    set_actor_identifier(actor, 6666);
    at<Byte>(actor, 84) = at<Byte>(actor, 372) = 1;
    at<Dword>(actor, 268) = 2;
    reset_animation_clock(offset(actor, 52));
    at<float>(owner, 36) = animation_duration(actor, 90902);
    at<Byte>(owner, 12) = 1;
}
} // namespace nfs::game::world::vehicle_render::pending
