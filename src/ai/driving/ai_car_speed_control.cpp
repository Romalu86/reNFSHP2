#include "ai_path_frame_x87.hpp"
#include "ai_car_behavior.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstring>
#include <cmath>
namespace nfs::game::runtime::ai::cars {

void __fastcall car_set_target_speed(void *entity, void *, Dword requested_word, Dword controls_word) {
    path_frame::set_target_speed(entity, requested_word, controls_word);
}
} // namespace nfs::game::runtime::ai::cars

namespace nfs::game::runtime::ai::cars::path_frame::legacy {
int truncate_control(const void *value) {
    const Dword bits = Dword(hardware::truncate_integer_native80(value));
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
}
