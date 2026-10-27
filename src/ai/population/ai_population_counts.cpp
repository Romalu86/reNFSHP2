#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/precision/extended_float.hpp"
#include "ai_population_x87.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
Dword chase_warmup_ticks{};

Byte advance_chase_warmup(void *chase) {
    if (at<Dword>(chase, 88) == 1 &&
        static_cast<std::int32_t>(chase_warmup_ticks) <
            static_cast<std::int32_t>(graphics::unresolved::camera_state -
                                      at<Dword>(chase, 24)))
        at<Dword>(chase, 88) = 2;
    return at<Dword>(chase, 88) == 1;
}

void recount_entity_kinds(Dword *owner) {
    population_frame::recount_entity_kinds(owner);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
