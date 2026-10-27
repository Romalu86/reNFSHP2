#include "arrest_cinematic_runtime.hpp"
#include <cstdio>
namespace nfs::game::world::actor_resources {
namespace {
int actor_count(void *owner) {
    return at<void *>(owner, 92) ? at<int>(at<void *>(owner, 92), 12) : 0;
}
Byte *selected_sequence(void *owner) {
    return at<Byte **>(owner, 88)[at<Dword>(owner, 80)];
}
} // namespace

void allocate_actor_sequences(void *owner, int count) {
    at<int>(owner, 84) = count;
    at<void *>(owner, 88) = files::archives::allocate_entry(
        int(Dword(count) * 4), "Character Sequence", lifecycle::character_pool);
    for (int sequence = 0; sequence < count; ++sequence) {
        at<void **>(owner, 88)[sequence] = files::archives::allocate_entry(
            actor_count(owner), "Character Sequence Actors", lifecycle::character_pool);
        at<int>(owner, 80) = sequence;
        for (int index = 0; index < actor_count(owner); ++index)
            selected_sequence(owner)[index] = 0;
    }
    at<Dword>(owner, 80) = 0;
}

void load_arrest_actors(void *owner) {
    char filename[260];
    const Dword variation = runtime::ai::random_word();
    std::sprintf(filename, "%sActorDef\\ArrestSeq%d.ini", bytes(subsystem(4)) + 784,
                 variation % 3 + 1);
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        int count = 0, sequence_count = 0;
        float position[3]{}, facing[3]{0, 0, 1};
        ini::get_int(&reader, "header", "numActors", &count);
        const char *position_keys[]{"X", "Y", "Z"}, *facing_keys[]{"DX", "DY", "DZ"};
        for (unsigned axis = 0; axis != 3; ++axis)
            ini::get_float(&reader, "header", position_keys[axis], position + axis);
        for (unsigned axis = 0; axis != 3; ++axis)
            ini::get_float(&reader, "header", facing_keys[axis], facing + axis);
        char offset_kind[32];
        ini::get_string(&reader, "header", "offsetType", offset_kind, 31);
        ini::get_int(&reader, "header", "numSequences", &sequence_count);
        for (int index = 0; index < count; ++index) {
            float actor_position[3], actor_facing[3];
            std::memcpy(actor_position, position, 12);
            std::memcpy(actor_facing, facing, 12);
            int animation = 0;
            void *actor = create_actor(owner, "CharacterSystem - LoadArrestActors");
            vehicle_render::allocate_animation_channels(offset(actor, 52),
                                                        "LoadArrest Characters animation channel",
                                                        3, lifecycle::character_pool);
            char section[32], resource[32];
            std::sprintf(section, "actor%d", Dword(index) + 1);
            ini::get_int(&reader, section, "animation", &animation);
            ini::get_string(&reader, section, "actorName", resource, 31);
            load_arrest_actor(actor, resource, Dword(animation),
                              {actor_position[0], actor_position[1], actor_position[2]},
                              {actor_facing[0], actor_facing[1], actor_facing[2]}, offset_kind);
            at<Byte>(actor, 61) = 0;
            at<Byte>(actor, 84) = 1;
        }
        if (!sequence_count) {
            allocate_actor_sequences(owner, 1);
            for (int index = 0; index < actor_count(owner); ++index)
                selected_sequence(owner)[index] = 1;
        } else {
            allocate_actor_sequences(owner, sequence_count);
            for (int sequence = 0; sequence < sequence_count; ++sequence) {
                char section[32];
                std::sprintf(section, "sequence%d", Dword(sequence) + 1);
                int entries = 0;
                ini::get_int(&reader, section, "numActors", &entries);
                at<int>(owner, 80) = sequence;
                for (int entry = 0; entry < entries; ++entry) {
                    char key[32];
                    int index = 0;
                    std::sprintf(key, "actorId%d", Dword(entry) + 1);
                    ini::get_int(&reader, section, key, &index);
                    selected_sequence(owner)[index] = 1;
                }
            }
        }
        bind_default_actor_events();
        ini::reader_clear(&reader);
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
}
} // namespace nfs::game::world::actor_resources
