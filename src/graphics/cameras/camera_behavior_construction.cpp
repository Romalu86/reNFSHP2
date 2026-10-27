#include "camera_director_runtime.hpp"
namespace nfs::game::world::camera {

void *construct_actor_transform(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    *behavior_words = reinterpret_cast<Dword>(actor_transform_methods);
    behavior_words[1] = 0;

    return behavior;
}

void *construct_actor_clip(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    *behavior_words = reinterpret_cast<Dword>(actor_clip_methods);
    behavior_words[1] = 0;

    return behavior;
}

void *construct_follow_fov(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    *behavior_words = reinterpret_cast<Dword>(follow_fov_methods);
    behavior_words[1] = 1112014848;
    behavior_words[2] = 1112014848;
    behavior_words[3] = 1112014848;
    behavior_words[9] = 0;
    *reinterpret_cast<Byte *>(behavior_words + 8) = 1;

    return behavior;
}

void *construct_follow_look(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    *behavior_words = reinterpret_cast<Dword>(follow_look_methods);
    behavior_words[1] = 0;
    behavior_words[2] = 0;
    behavior_words[33] = 0;

    return behavior;
}

void *construct_mount_look(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    *behavior_words = reinterpret_cast<Dword>(mount_look_methods);
    behavior_words[1] = 0;
    behavior_words[5] = 0;
    behavior_words[4] = 0;
    behavior_words[2] = 0;
    behavior_words[3] = 1065353216;

    return behavior;
}

void *construct_point_look(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    *behavior_words = reinterpret_cast<Dword>(point_look_methods);

    return behavior;
}

void *construct_follow_position(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    behavior_words[2] = 0;
    behavior_words[3] = 0;
    behavior_words[4] = 0;
    behavior_words[5] = 0;
    behavior_words[6] = 0;
    behavior_words[7] = 0;
    behavior_words[18] = 0;
    behavior_words[19] = 0;
    behavior_words[20] = 0;
    behavior_words[15] = behavior_words[18];
    behavior_words[16] = behavior_words[19];
    behavior_words[17] = behavior_words[20];
    behavior_words[8] = 0;
    *reinterpret_cast<std::uint16_t *>(behavior_words + 21) = 0;
    *reinterpret_cast<std::uint16_t *>(reinterpret_cast<Dword>(behavior_words) + 86) = 0;
    *reinterpret_cast<std::uint16_t *>(behavior_words + 22) = 0;
    behavior_words[12] = behavior_words[18];
    behavior_words[13] = behavior_words[19];
    behavior_words[14] = behavior_words[20];
    behavior_words[9] = behavior_words[18];
    behavior_words[10] = behavior_words[19];
    behavior_words[11] = behavior_words[20];
    behavior_words[94] = 0;
    behavior_words[95] = 0;
    behavior_words[96] = 0;
    behavior_words[99] = 0;
    behavior_words[100] = 0;
    behavior_words[101] = 0;
    behavior_words[102] = 0;
    behavior_words[103] = 0;
    behavior_words[104] = 0;
    behavior_words[105] = 0;
    behavior_words[106] = 0;
    behavior_words[107] = 0;
    behavior_words[108] = 0;
    behavior_words[109] = 0;
    behavior_words[110] = 0;
    behavior_words[112] = 0;
    behavior_words[113] = 0;
    behavior_words[114] = 0;
    behavior_words[115] = 0;
    behavior_words[116] = 0;
    behavior_words[117] = 0;
    *reinterpret_cast<Byte *>(behavior_words + 123) = 0;
    *behavior_words = reinterpret_cast<Dword>(follow_position_methods);
    behavior_words[1] = 0;
    behavior_words[23] = 1112014848;
    *reinterpret_cast<Byte *>(behavior_words + 24) = 0;
    behavior_words[65] = 0;
    behavior_words[99] = 0;
    behavior_words[100] = 0;
    behavior_words[101] = 0;
    behavior_words[102] = 0;
    behavior_words[31] = 1069547520;
    behavior_words[32] = 1053609165;
    *reinterpret_cast<Byte *>(reinterpret_cast<Dword>(behavior_words) + 97) = 0;
    behavior_words[66] = 1092616192;
    behavior_words[67] = 1097859072;
    *reinterpret_cast<Byte *>(behavior_words + 68) = 1;
    behavior_words[103] = 0;
    behavior_words[104] = 0;
    behavior_words[105] = 0;
    behavior_words[106] = 0;
    behavior_words[107] = 0;
    behavior_words[108] = 0;
    behavior_words[109] = 0;
    behavior_words[110] = 0;
    behavior_words[111] = 0;
    behavior_words[118] = 0;
    behavior_words[119] = 0;
    behavior_words[120] = 0;
    behavior_words[121] = 0;
    behavior_words[122] = 0;

    return behavior;
}

void *construct_matrix_position(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    Dword *history;
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    behavior_words[9] = 0;
    behavior_words[10] = 0;
    behavior_words[11] = 0;
    behavior_words[12] = 0;
    behavior_words[13] = 0;
    behavior_words[14] = 0;
    history = behavior_words + 25;
    *history = 0;
    behavior_words[26] = 0;
    behavior_words[27] = 0;
    behavior_words[22] = *history;
    behavior_words[23] = behavior_words[26];
    behavior_words[24] = behavior_words[27];
    behavior_words[19] = *history;
    behavior_words[20] = behavior_words[26];
    behavior_words[21] = behavior_words[27];
    behavior_words[16] = *history;
    behavior_words[17] = behavior_words[26];
    behavior_words[18] = behavior_words[27];
    behavior_words[15] = 0;
    *reinterpret_cast<std::uint16_t *>(behavior_words + 28) = 0;
    *reinterpret_cast<std::uint16_t *>(reinterpret_cast<Dword>(behavior_words) + 114) = 0;
    *reinterpret_cast<std::uint16_t *>(behavior_words + 29) = 0;
    *behavior_words = reinterpret_cast<Dword>(matrix_position_methods);
    behavior_words[2] = 3196059648u;
    behavior_words[3] = 1008981770;
    behavior_words[4] = 0;
    behavior_words[8] = 0;
    behavior_words[7] = 0;
    behavior_words[5] = 0;
    behavior_words[6] = 1065353216;

    return behavior;
}

void *construct_mount_position(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    Dword *history;
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    behavior_words[9] = 0;
    behavior_words[10] = 0;
    behavior_words[11] = 0;
    behavior_words[12] = 0;
    behavior_words[13] = 0;
    behavior_words[14] = 0;
    history = behavior_words + 25;
    *history = 0;
    behavior_words[26] = 0;
    behavior_words[27] = 0;
    behavior_words[22] = *history;
    behavior_words[23] = behavior_words[26];
    behavior_words[24] = behavior_words[27];
    behavior_words[19] = *history;
    behavior_words[20] = behavior_words[26];
    behavior_words[21] = behavior_words[27];
    behavior_words[16] = *history;
    behavior_words[17] = behavior_words[26];
    behavior_words[18] = behavior_words[27];
    behavior_words[15] = 0;
    *reinterpret_cast<std::uint16_t *>(behavior_words + 28) = 0;
    *reinterpret_cast<std::uint16_t *>(reinterpret_cast<Dword>(behavior_words) + 114) = 0;
    *reinterpret_cast<std::uint16_t *>(behavior_words + 29) = 0;
    *behavior_words = reinterpret_cast<Dword>(mount_position_methods);
    behavior_words[1] = 0;
    behavior_words[4] = 0;
    behavior_words[2] = 0;
    behavior_words[3] = 1065353216;

    return behavior;
}

void *construct_pause_position(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    Dword *history;
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    behavior_words[7] = 0;
    behavior_words[8] = 0;
    behavior_words[9] = 0;
    behavior_words[10] = 0;
    behavior_words[11] = 0;
    behavior_words[12] = 0;
    history = behavior_words + 23;
    *history = 0;
    behavior_words[24] = 0;
    behavior_words[25] = 0;
    behavior_words[20] = *history;
    behavior_words[21] = behavior_words[24];
    behavior_words[22] = behavior_words[25];
    behavior_words[17] = *history;
    behavior_words[18] = behavior_words[24];
    behavior_words[19] = behavior_words[25];
    behavior_words[14] = *history;
    behavior_words[15] = behavior_words[24];
    behavior_words[16] = behavior_words[25];
    behavior_words[13] = 0;
    *reinterpret_cast<std::uint16_t *>(behavior_words + 26) = 0;
    *reinterpret_cast<std::uint16_t *>(reinterpret_cast<Dword>(behavior_words) + 106) = 0;
    *reinterpret_cast<std::uint16_t *>(behavior_words + 27) = 0;
    *behavior_words = reinterpret_cast<Dword>(pause_position_methods);
    behavior_words[1] = 0;
    behavior_words[2] = 985963430;
    behavior_words[6] = 0;
    behavior_words[5] = 0;
    behavior_words[3] = 0;
    behavior_words[4] = 1065353216;

    return behavior;
}

void *construct_follow_up(void *behavior) {
    auto *behavior_words = static_cast<Dword *>(behavior);
    *behavior_words = reinterpret_cast<Dword>(position_base_methods);
    *behavior_words = reinterpret_cast<Dword>(follow_up_methods);
    behavior_words[1] = 0;
    behavior_words[4] = 0;
    behavior_words[2] = 0;
    behavior_words[3] = 1065353216;
    behavior_words[5] = 0;
    behavior_words[6] = 1065353216;

    return behavior;
}
} // namespace nfs::game::world::camera
