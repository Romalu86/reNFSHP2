#include "car_construction_runtime.hpp"
namespace nfs::game::runtime::physics::construction {

void *fill_car_history(void *history, const Dword *initial) {
    const Dword value = *initial;
    for (unsigned word = 0; word < 4; ++word)
        at<Dword>(history, word * 4) = value;
    return history;
}

void clear_car_run_statistics(void *statistics) {
    for (unsigned field = 4; field <= 124; field += 4)
        at<Dword>(statistics, field) = 0;
}

void *construct_car_run_statistics(void *statistics) {
    at<Byte>(statistics, 0) = 0;
    clear_car_run_statistics(statistics);
    return statistics;
}

void *construct_impact_state(void *state, void *car) {
    at<void *>(state, 28) = car;
    at<Byte>(state, 0) = 0;
    at<Dword>(state, 4) = 4294967295u;
    for (unsigned field = 8; field <= 24; field += 4)
        at<Dword>(state, field) = 0;
    return state;
}

void __fastcall query_car_position(void *car, void *, float *position) {
    void *body = at<void *>(car, 1768);
    const Dword x = at<Dword>(body, 680);
    at<Dword>(position, 0) = x;
    const Dword y = at<Dword>(body, 684), z = at<Dword>(body, 688);
    at<Dword>(position, 4) = y;
    at<Dword>(position, 8) = z;
}
const Dword car_position_methods[] = {reinterpret_cast<Dword>(query_car_position)};
const Dword car_base_methods[] = {
    reinterpret_cast<Dword>(assertions::pure_virtual_call)};

void initialize_wheel_state(void *wheel, void *suspension, void *dynamics, Dword index, void *car) {
    auto *wheel_words = static_cast<Dword *>(wheel);
    wheel_words[1] = reinterpret_cast<Dword>(suspension);
    *wheel_words = reinterpret_cast<Dword>(car);
    wheel_words[3] = index;
    wheel_words[2] = reinterpret_cast<Dword>(dynamics);
    wheel_words[4] = 0;
    wheel_words[5] = 0;
    wheel_words[6] = 0;
    wheel_words[7] = 0;
    wheel_words[8] = 0;
    wheel_words[9] = 0;
    wheel_words[10] = 0;
    wheel_words[11] = 0;
    kernel::memory_zero(wheel_words + 12, 84);
    wheel_words[33] = 1;
    kernel::memory_zero(wheel_words + 34, 8);
    wheel_words[36] = 0;
    wheel_words[37] = 0;
    wheel_words[38] = 0;
    wheel_words[39] = 0;
    wheel_words[40] = 0;
    wheel_words[41] = 0;
    wheel_words[42] = 0;
    wheel_words[43] = 0;
    wheel_words[44] = 0;
    wheel_words[45] = 0;
    wheel_words[46] = 0;
    wheel_words[47] = 0;
    wheel_words[48] = 0;
    wheel_words[49] = 0;
    wheel_words[50] = 0;
    wheel_words[57] = 0;
    wheel_words[58] = 0;
    wheel_words[59] = 0;
    wheel_words[60] = 0;
    wheel_words[61] = 0;
    wheel_words[62] = 0;
    wheel_words[69] = 0;
    wheel_words[70] = 0;
    wheel_words[51] = 0;
    wheel_words[52] = 0;
    wheel_words[53] = 0;
    wheel_words[54] = 0;
    *reinterpret_cast<Byte *>(wheel_words + 55) = 0;
    *reinterpret_cast<Byte *>(reinterpret_cast<Dword>(wheel_words) + 221) = 0;
    wheel_words[56] = 0;
    wheel_words[63] = 0;
    *reinterpret_cast<Byte *>(wheel_words + 64) = 0;
    wheel_words[65] = 0;
    wheel_words[66] = 0;
    wheel_words[67] = 0;
    wheel_words[68] = 0;
    wheel_words[71] = 0;
    wheel_words[72] = 0;
    wheel_words[73] = 0;
    return;
}

} // namespace nfs::game::runtime::physics::construction
