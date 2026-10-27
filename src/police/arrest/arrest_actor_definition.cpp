#include "arrest_cinematic_runtime.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include <cstdio>
namespace nfs::game::world::actor_resources {

// Pass the position and facing triples by value.
void load_arrest_actor(void *actor, const char *name, Dword animation, ActorVector position,
                       ActorVector facing, const char *offset_kind) {
    char filename[264];
    std::sprintf(filename, "%sActorDef\\%s.adf", bytes(subsystem(4)) + 784, name);
    if (!files::file::exists(filename))
        return;
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        int model_number = 0, texture_number = 0;
        char resource[32], model[32], texture[32];
        ini::get_int(&reader, "actor", "modelNumber", &model_number);
        ini::get_int(&reader, "actor", "textureNumber", &texture_number);
        ini::get_string(&reader, "actor", "actorName", resource, 31);
        const auto format = kernel::numeric::current_format();
        float frame_rate = float(kernel::numeric::Extended(at<float>(actor, 376), format) * 60.f);
        ini::get_float(&reader, "actor", "fps", &frame_rate);
        at<float>(actor, 376) =
            float(kernel::numeric::Extended(frame_rate, format) * 0.01666666753590106964111328125f);
        std::sprintf(model, "Model%d", model_number);
        std::sprintf(texture, "Tex%d.fsh", texture_number);
        if (void *child = at<void *>(actor, 264))
            yaw_actor(child, 0.f);
        float rotation[16], direction[3];
        actor_axis_rotation(rotation, 0.f, 0.f, 1.f, 0.f);
        std::memcpy(direction, bytes(actor) + 136, 12);
        graphics::math::transform_position(direction, rotation, direction);
        runtime::normalize_vector(direction, direction);
        std::memcpy(offset(actor, 136), direction, 12);
        at<float>(actor, 176) = 10000.f;
        at<Dword>(actor, 172) = 9999;
        std::sprintf(reinterpret_cast<char *>(offset(actor, 416)), "actor%d", 9999);
        std::memcpy(offset(actor, 124), &position, sizeof position);
        std::memcpy(offset(actor, 136), &facing, sizeof facing);
        bind_actor_resources(actor, resource, model, texture, 1, 0, 0);
        at<Dword>(actor, 268) = *offset_kind == 'a' || *offset_kind == 'A'   ? 2
                                 : *offset_kind == 'd' || *offset_kind == 'D' ? 1
                                                                              : 4;
        vehicle_render::pending::select_character_animation(actor, animation, 0);
        runtime::physics::clear_ground_contact(offset(actor, 180));
        ini::reader_clear(&reader);
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
}
} // namespace nfs::game::world::actor_resources
