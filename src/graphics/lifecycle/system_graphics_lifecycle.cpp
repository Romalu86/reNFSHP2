#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/graphics/backend/eagl_state.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstring>
namespace nfs::lifecycle::table_entries {
namespace {
Dword *fields(System *object) {
    return reinterpret_cast<Dword *>(object);
}
std::uint8_t &byte(System *object, unsigned offset) {
    return reinterpret_cast<std::uint8_t *>(object)[offset];
}
} // namespace

void __fastcall initialize_lod(System *object, void *) {
    for (unsigned i = 4; i < 8; ++i)
        byte(object, i) = 1;
    fields(object)[2] = 1036831949;
    byte(object, 12) = byte(object, 13) = byte(object, 14) = 1;
    fields(object)[4] = 1056964608;
    byte(object, 20) = 1;
    construction::lod_settings(fields(object) + 6);
}

void __fastcall initialize_fog(System *object, void *) {
    if (graphics::context) {
        auto *state = fields(object);
        byte(object, 112) = 0;
        graphics::state::set_context_fog_enabled(graphics::context, 0);
        graphics::backend::set_table_fog(graphics::context, state[25]);
        graphics::backend::set_vertex_fog(graphics::context, state[26]);
        Dword colour = (((state[18] & 255) | (state[17] << 8)) << 8 | (state[19] & 255)) << 8 |
                      (state[20] & 255);
        graphics::backend::set_fog_colour(graphics::context, colour);
        float density;
        std::memcpy(&density, state + 23, 4);
        graphics::backend::set_fog_density(graphics::context, density);
        graphics::state::set_fog_start(graphics::context, state[21]);
        graphics::state::set_fog_end(graphics::context, state[22]);
        byte(object, 113) = 1;
    }
}

void __fastcall load_fog_level(System *object, void *) {
    auto *rules = reinterpret_cast<const Dword *>(manager->slots[72 / 4]);
    byte(object, 113) = static_cast<int>(rules[2388 / 4]) > 3;
}
} // namespace nfs::lifecycle::table_entries
