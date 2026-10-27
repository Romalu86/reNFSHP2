#include "race_render_passes.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <initializer_list>
namespace nfs::game::world::fog {
using game::runtime::at;
using kernel::Dword;

void begin(void *fog) {
    at<std::uint8_t>(fog, 112) = 1;
}

void end(void *fog) {
    at<std::uint8_t>(fog, 112) = 0;
    graphics::state::set_context_fog_enabled(graphics::context, 0);
}

void apply(void *fog, std::uint8_t scaled, std::uint8_t requested) {
    if (!requested || !at<std::uint8_t>(fog, 112) || !at<std::uint8_t>(fog, 113)) {
        graphics::state::set_context_fog_enabled(graphics::context, 0);
        return;
    }
    graphics::state::set_context_fog_enabled(graphics::context, 1);
    graphics::backend::set_table_fog(graphics::context, 0);
    graphics::backend::set_vertex_fog(graphics::context, 2);
    Dword colour = at<Dword>(fog, 68);
    for (unsigned offset : {72u, 76u, 80u})
        colour = (colour << 8) | (at<Dword>(fog, offset) & 255);
    graphics::backend::set_fog_colour(graphics::context, colour);
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    const float density = at<float>(fog, 92);
    float result = scaled ? float(Extended(at<float>(fog, 96), format) * density * 0.039999999105930328369140625f)
                          : float(Extended(density, format) * 0.00999999977648258209228515625f);
    if (result < 0)
        result = 0;
    else if (result > 1)
        result = 1;
    graphics::backend::set_fog_density(graphics::context, result);
}
} // namespace nfs::game::world::fog
