#include "../resources/track_render_resources.hpp"
#include "../../physics/bodies/shape_body_runtime.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstring>
#include <cmath>
namespace nfs::game::world::dependency {

Dword track_reset_callback_disabled = 3524332777u, track_reset_callback = 2773448519u;

Byte project_boom_to_ground(const float *position, Dword *surface, float *output, float *normal) {
    alignas(4) Byte contact[84];
    runtime::physics::clear_ground_contact(contact);
    output[2] = -position[2];
    const Dword x = at<Dword>(position, 0), y = at<Dword>(position, 4);
    *surface = 0;
    at<Dword>(output, 0) = x;
    at<Dword>(output, 4) = y;
    if (runtime::physics::sample_ground(output, contact)) {
        const auto format = kernel::numeric::current_format();
        using kernel::numeric::Extended;
        auto X = [&](float value) { return Extended(value, format); };
        const auto x_difference = X(output[0]) - at<float>(contact, 0);
        *surface = at<Dword>(contact, 24);
        const auto x_slope = x_difference * at<float>(contact, 12);
        const auto z = X(output[2]);
        normal[1] = at<float>(contact, 16);
        const auto z_slope = (z - at<float>(contact, 8)) * at<float>(contact, 20);
        output[1] = float(X(at<float>(contact, 4)) -
                          (x_slope + z_slope) /
                              X(at<float>(contact, 16)));
        normal[0] = at<float>(contact, 12);
        normal[2] = -at<float>(contact, 20);
        output[2] = -output[2];
        return 1;
    }
    const float z = output[2];
    *surface = 0;
    normal[0] = 0;
    normal[2] = 0;
    output[2] = -z;
    normal[1] = 1;
    return 0;
}

void bind_boom_model(void *level, Dword *output, const char *name) {
    eagl::SymbolView view;
    track_render::find_model(&view, at<eagl::Loader *>(level, 20), name, 0);
    if (view.address) {
        *output = view.address;
        track_render::configure_boom_states(reinterpret_cast<void *>(view.address));
    } else
        *output = 0;
}

void bind_boom_material(void *owner, const char *name) {
    at<Dword>(owner, 100) = at<Dword>(owner, 108) = at<Dword>(owner, 104) = Dword(-1);
    if (!name || !_stricmp(name, &runtime::particles::lifecycle::state<char>(7217172)))
        return;
    for (unsigned index = 0; index < 30; ++index) {
        Byte *record = services::pending::boom_sound_definitions + index * 312;
        if (!_stricmp(name, reinterpret_cast<char *>(record))) {
            at<Dword>(owner, 100) = at<Dword>(record, 260);
            if (at<Dword>(record, 280) != Dword(-1))
                at<Dword>(owner, 108) = index;
            if (at<Dword>(record, 296) != Dword(-1))
                at<Dword>(owner, 104) = index;
            return;
        }
    }
}
namespace {
void set_ground_delay(void *owner, void *definition, void *properties, float *position) {
    void *channels = at<void *>(properties, 8);
    at<Byte>(owner, 96) = 0;
    if (channels && at<Dword>(channels, 0)) {
        at<Dword>(owner, 84) = 0;
        return;
    }
    const auto format = kernel::numeric::current_format();
    using kernel::numeric::Extended;
    at<Dword>(owner, 92) = 0;
    void *material = at<void *>(properties, 0);
    at<float>(owner, 84) = float(Extended(at<float>(material, 44), format) * 1000.f);
    if (!at<std::int16_t>(definition, 28)) {
        float normal[3];
        if (project_boom_to_ground(position, &at<Dword>(owner, 92), position, normal))
            position[1] = float(Extended(at<float>(material, 60), format) + position[1]);
        else
            at<Byte>(owner, 96) = 1;
    }
}
} // namespace
namespace pending {

void initialize_track_object(void *owner, int index) {
    void *definition =
        offset(at<void *>(world::pending::gob_definitions, 60), Dword(index) * 36);
    at<void *>(owner, 76) = definition;
    if (at<std::int16_t>(definition, 28) == 2)
        return;
    void *properties = at<void *>(definition, 32), *material = at<void *>(properties, 0);
    float mass = at<float>(material, 40);
    at<float>(material, 40) = mass > 25.f ? mass : 25.f; // TEST AH,41 also selects25 for NaN.
    if (const char *name = at<const char *>(properties, 4))
        bind_boom_model(subsystem(24), &at<Dword>(owner, 88), name);
    else
        at<Dword>(owner, 88) = 0;
    set_ground_delay(owner, definition, properties,
                     reinterpret_cast<float *>(offset(definition, 16)));
    std::memcpy(offset(owner, 28), offset(definition, 16), 12);
    world::quaternion_matrix(static_cast<const float *>(definition),
                             reinterpret_cast<float *>(offset(owner, 40)));
    bind_boom_material(owner, reinterpret_cast<const char *>(offset(material, 80)));
    if (at<Dword>(owner, 100) != Dword(-1))
        at<Dword>(material, 52) = at<Dword>(owner, 100);
    at<Dword>(owner, 16) = runtime::race_audio::next_emitter_serial();
}
} // namespace pending

void reset_track_object(void *owner) {
    if (at<void *>(owner, 80))
        runtime::physics::reset_track_vehicle(owner);
    void *definition = at<void *>(owner, 76);
    if (!definition)
        return;
    std::memcpy(offset(owner, 28), offset(definition, 16), 12);
    world::quaternion_matrix(static_cast<const float *>(definition),
                             reinterpret_cast<float *>(offset(owner, 40)));
    set_ground_delay(owner, definition, at<void *>(definition, 32),
                     reinterpret_cast<float *>(offset(owner, 28)));
    at<Byte>(owner, 116) = 0;
}
} // namespace nfs::game::world::dependency
namespace nfs::lifecycle::phase_dependencies {

void finalize_vehicle_load() {
    using namespace game::world;
    using game::runtime::track_collision;
    const int count = at<int>(track_collision, 4);
    for (int index = 0; index < count; ++index) {
        Dword displacement = Dword(index) * 120;
        void *object = int(displacement) >= 0 && index < at<int>(track_collision, 4)
                           ? offset(at<void *>(track_collision, 16), displacement)
                           : nullptr;
        dependency::reset_track_object(object);
    }
    dependency::release_track_sky();
    game::world::pending::initialize_track_collision_grid();
    load_gob_database(track_collision);
    if (!dependency::track_reset_callback_disabled)
        reinterpret_cast<void(__cdecl *)()>(dependency::track_reset_callback)();
}
} // namespace nfs::lifecycle::phase_dependencies
