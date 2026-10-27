#include "car_construction_runtime.hpp"
namespace nfs::game::runtime::physics::construction {

void *construct_scalar_parameters(void *parameters) {
    auto *words = static_cast<Dword *>(parameters);

    *words = 0;
    words[1] = 0;
    words[2] = 0;
    words[3] = 0;
    words[4] = 0;
    words[5] = 0;
    words[6] = 0;
    words[7] = 0;
    words[8] = 0;
    words[9] = 0;
    words[13] = 0;
    words[12] = 0;
    words[11] = 0;
    words[10] = 0;
    words[14] = 0;
    words[15] = 0;
    return parameters;
}

void *construct_response_parameters(void *parameters) {
    auto *words = static_cast<Dword *>(parameters);

    words[6] = 0;
    words[5] = 0;
    words[4] = 0;
    words[3] = 0;
    words[2] = 0;
    words[1] = 0;
    *words = 0;
    words[8] = 0;
    words[7] = 0;
    return parameters;
}

void *construct_curve_parameters(void *parameters) {
    auto *words = static_cast<Dword *>(parameters);

    words[6] = 0;
    words[7] = 0;
    words[8] = 0;
    words[9] = 0;
    words[10] = 0;
    words[1] = 0;
    *words = 0;
    words[3] = 0;
    words[2] = 0;
    words[4] = 0;
    words[5] = 0;
    words[11] = 0;
    kernel::memory_zero(words + 12, 32);
    kernel::memory_zero(words + 20, 32);
    kernel::memory_zero(words + 28, 32);
    words[36] = 0;
    words[37] = 0;
    words[38] = 0;
    words[39] = 0;
    words[42] = 0;
    words[41] = 0;
    words[40] = 0;
    kernel::memory_zero(words + 43, 32);
    kernel::memory_zero(words + 51, 32);
    words[60] = 0;
    words[59] = 0;
    words[61] = 0;
    kernel::memory_zero(words + 62, 8);
    words[64] = 0;
    return parameters;
}

void *construct_pose_parameters(void *parameters) {
    auto *words = static_cast<Dword *>(parameters);
    int word_count;
    Dword *source;
    Dword *destination;

    const Dword identity[9] = {1065353216, 0, 0, 0, 1065353216, 0, 0, 0, 1065353216};
    source = const_cast<Dword *>(identity);
    destination = words + 3;
    for (word_count = 9; word_count != 0; word_count = word_count + -1) {
        *destination = *source;
        source = source + 1;
        destination = destination + 1;
    }
    source = const_cast<Dword *>(identity);
    destination = words + 12;
    for (word_count = 9; word_count != 0; word_count = word_count + -1) {
        *destination = *source;
        source = source + 1;
        destination = destination + 1;
    }
    words[2] = 0;
    words[1] = 0;
    *words = 0;
    words[21] = 0;
    words[22] = 0;
    words[24] = 0;
    words[23] = 0;
    words[26] = 0;
    words[27] = 0;
    words[25] = 0;
    return parameters;
}

void construct_control_state(void *parameters) {
    auto *words = static_cast<Dword *>(parameters);

    *words = 0;
    words[1] = 0;
    words[2] = 0;
    words[3] = 0;
    words[4] = 0;
    words[5] = 0;
    words[6] = 0;
    words[7] = 0;
    words[8] = 0;
    words[9] = 0;
    words[10] = 0;
    words[11] = 0;
    words[12] = 0;
    words[13] = 0;
    words[14] = 0;
    words[15] = 0;
    words[16] = 0;
    words[17] = 0;
    kernel::memory_zero(words + 20, 16);
    kernel::memory_zero(reinterpret_cast<Byte *>(words) + 75, 4);
    kernel::memory_zero(words + 25, 48);
    kernel::memory_zero(words + 24, 4);
    kernel::memory_zero(words + 18, 3);
    return;
}
} // namespace nfs::game::runtime::physics::construction
