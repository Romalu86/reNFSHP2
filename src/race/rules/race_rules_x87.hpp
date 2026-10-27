#pragma once

namespace nfs::game::runtime::ai::race_progress_x87 {
void priority_native80(void *vehicle, void *output);
}

#if defined(__clang__) && __LDBL_MANT_DIG__ == 64
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../ai/population/ai_population_x87.hpp"
#include <cstring>

namespace nfs::game::runtime::ai::race_progress_x87 {
inline long double priority(void *vehicle) {
    long double value;
    priority_native80(vehicle, &value);
    return value;
}
}

namespace nfs::application::race_rules_x87 {
inline long double retained(float value) { return static_cast<long double>(value); }
inline kernel::Dword truncate(long double value) {
    return game::runtime::ai::lifecycle::pending::population_frame::truncate_native80(&value);
}
}
#endif
