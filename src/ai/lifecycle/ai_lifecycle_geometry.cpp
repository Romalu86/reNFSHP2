#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"

namespace nfs::game::runtime::ai::lifecycle {

void rotate_y_basis(const float *input, float angle, float *output) {
    float copy[9];
    if (input == output) {
        for (unsigned index = 0; index < 9; ++index)
            copy[index] = input[index];
        input = copy;
    }
    float sine, cosine;
    turn_sine_cosine(&angle, &sine, &cosine);
    for (unsigned index = 0; index < 3; ++index)
        output[index] = float(static_cast<long double>(cosine) * input[index] -
                              static_cast<long double>(sine) * input[6 + index]);
    for (unsigned index = 3; index < 6; ++index)
        output[index] = input[index];
    for (unsigned index = 0; index < 3; ++index)
        output[6 + index] = float(static_cast<long double>(cosine) * input[6 + index] +
                                  static_cast<long double>(sine) * input[index]);
}

long double random_fraction() {
    ++pending::fractional_random_calls;
    const Dword low = random_state[4] + random_state[5];
    Dword carry = low < random_state[5] || low < random_state[4];
    Dword value = low + carry + random_state[3];
    carry = value < random_state[3];
    random_state[4] = low;
    Dword next = value + carry + random_state[2];
    carry = next < random_state[2];
    random_state[3] = value;
    value = next + carry + random_state[1];
    carry = value < random_state[1];
    random_state[2] = next;
    random_state[1] = value;
    Dword result = random_state[0] + value + carry;
    ++random_state[5];
    random_state[0] = result;
    if (!random_state[5] && !++random_state[4] && !++random_state[3] && !++random_state[2] &&
        !++random_state[1])
        random_state[0] = ++result;
    return static_cast<long double>(result & 65535u) * 0.0000152587890625L;
}
} // namespace nfs::game::runtime::ai::lifecycle
