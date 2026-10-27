#include "character_runtime.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::actor_resources {

void load_actor_definition(void *actor, const void *definition) {
    char filename[260];
    std::sprintf(filename, "%sActorDef\\%s.adf", bytes(subsystem(4)) + 784,
                 bytes(definition) + 28);
    at<Dword>(actor, 124) = 3270508544u;
    at<Dword>(actor, 128) = 3264217088u;
    at<Dword>(actor, 132) = 1142456320;
    if (!files::file::exists(filename))
        return;
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        int model_number = 0, texture_number = 0, animation = 0, sound_id = 0;
        float scale = 1;
        ini::get_int(&reader, "actor", "modelNumber", &model_number);
        ini::get_int(&reader, "actor", "textureNumber", &texture_number);
        ini::get_int(&reader, "actor", "defaultAnimation", &animation);
        ini::get_int(&reader, "actor", "soundID", &sound_id);
        ini::get_float(&reader, "actor", "scale", &scale);
        at<float>(actor, 120) = scale;
        char resource[32], model[32], texture[32];
        // The saved missing actorName path leaves this buffer unwritten.
        ini::get_string(&reader, "actor", "actorName", resource, 31);
        std::sprintf(model, "Model%d", model_number);
        std::sprintf(texture, "Tex%d.fsh", texture_number);
        const int sound = find_actor_sound(sound_id);
        at<int>(actor, 96) = sound;
        at<Dword>(actor, 100) = sound == -1 ? 0 : runtime::race_audio::next_emitter_serial();
        configure_actor_behavior(actor, resource, model, texture, Dword(animation), definition);
        ini::reader_clear(&reader);
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
}

void configure_actor_behavior(void *actor, const char *resource, const char *model,
                              const char *texture, Dword animation, const void *definition) {
    const int kind = at<int>(definition, 12);
    auto bind = [&](ActorAnimationCallback animation_callback, ActorFrameCallback frame_callback) {
        bind_actor_resources(actor, resource, model, texture, 1, animation_callback, frame_callback);
    };
    auto select = [&](Dword identifier) {
        vehicle_render::pending::select_character_animation(actor, identifier, 0);
    };
    auto path = [&](Byte use_marker_position) {
        reset_animation_clock(offset(actor, 52));
        const int index = at<int>(definition, 16);
        void *catalog = runtime::track_collision;
        void *record = index >= 0 && index < at<int>(catalog, 8)
                           ? bytes(at<void *>(catalog, 20)) + Dword(index) * 72
                           : nullptr;
        bind_path_cursor(offset(actor, 36), record);
        float position[3];
        if (use_marker_position) {
            position[0] = at<float>(definition, 108);
            position[1] = at<float>(definition, 112);
            position[2] = -at<float>(definition, 116);
        } else {
            position[0] = at<float>(actor, 124);
            position[1] = at<float>(actor, 128);
            position[2] = -at<float>(actor, 132);
        }
        locate_path_cursor(offset(actor, 36), position);
    };
    if (kind == 10 || kind == 11 || kind == 20 || kind == 21) {
        bind(kind >= 20 ? freeze_actor_channel_offsets : nullptr, nullptr);
        at<Dword>(actor, 268) = kind == 10 || kind == 20 ? 2 : 4;
        select(animation);
        if (kind < 20)
            reset_animation_clock(offset(actor, 52));
        return;
    }
    if (kind == 1006) {
        bind(0, 0);
        at<Dword>(actor, 268) = 4;
        select(10402);
        reset_animation_clock(offset(actor, 52));
        return;
    }
    if (kind == 1000) {
        bind(nullptr, update_actor_walking_path);
        at<Dword>(actor, 268) = 1;
        select(10101);
        path(0);
        return;
    }
    if (kind == 1009) {
        bind(nullptr, update_actor_continuous_path);
        at<Dword>(actor, 268) = 1;
        select(10404);
        path(1);
        return;
    }
    if (kind == 1004)
        bind(nullptr, update_actor_proximity_loop);
    else if (kind == 1007) {
        bind(nullptr, update_actor_proximity_pose);
        animation = 10401;
    } else if (kind == 1008) {
        bind(nullptr, update_actor_proximity_motion);
        animation = 20408;
    } else if (kind == 1010 || kind == 1011) {
        bind(nullptr, update_actor_proximity_transition);
        animation = kind == 1010 ? 10103 : 10105;
    } else {
        resource = "BipedA";
        model = "Model1";
        texture = "Tex1.fsh";
        bind(nullptr, update_actor_proximity_loop);
        animation = 10101;
    }
    if (kind == 1004)
        animation = 10101;
    at<Dword>(actor, 268) = 1;
    select(animation);
    reset_animation_clock(offset(actor, 52));
}
} // namespace nfs::game::world::actor_resources
namespace nfs::game::world::pending {

void *level_actor_definition(void *owner, int index) {
    return bytes(at<void *>(owner, 152)) + Dword(index) * 124;
}

void load_character_assets(void *owner, int count) {
    using namespace actor_resources;
    load_actor_sound_catalog();
    for (int index = 0; index < count; ++index) {
        void *actor = create_actor(owner, "CharacterSystem - InitActor");
        vehicle_render::allocate_animation_channels(
            offset(actor, 52), "InitActors - animation channels", 3, lifecycle::character_pool);
        const void *definition = level_actor_definition(subsystem(24), index);
        yaw_actor(actor, at<float>(definition, 120));
        const float radius = at<float>(definition, 20);
        at<float>(actor, 176) = radius < 50 ? 50.f : radius;
        set_actor_identifier(actor, at<Dword>(definition, 0));
        std::memcpy(offset(actor, 124), bytes(definition) + 108, 12);
        load_actor_definition(actor, definition);
        runtime::physics::clear_ground_contact(offset(actor, 180));
    }
    bind_default_actor_events();
}
} // namespace nfs::game::world::pending
