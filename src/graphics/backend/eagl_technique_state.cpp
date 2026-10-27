#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::techniques {
namespace {
void constants(Dword *technique, Dword state_address, bool apply_texture_delta) {
    for (int index = 0; index < static_cast<int>(technique[0]); ++index) {
        auto *assignments = reinterpret_cast<Dword *>(technique[1]);
        auto *assignment = reinterpret_cast<Dword *>(assignments[index]);
        switch (assignment[2]) {
        case 3: {
            Dword texture_address = backend::unresolved::texture_cache[assignment[3]];
            if (texture_address) {
                auto *texture = reinterpret_cast<Dword *>(texture_address);
                tar::set_stage(texture + 10, assignment[1]);

                tar::bind(reinterpret_cast<Dword *>(
                    backend::unresolved::texture_cache[assignment[3]]));
            }
            break;
        }
        case 4:
            if (apply_texture_delta)
                backend::apply_texture_delta(
                    reinterpret_cast<backend::TextureStageState *>(assignment[0]));
            break;
        case 5:
            *reinterpret_cast<Dword *>(state_address + assignment[0]) = assignment[3];
            break;
        case 6:
        case 8:
            resources::bind_vertex(reinterpret_cast<Dword *>(assignment[3]));
            break;
        case 7:
            resources::bind_pixel(reinterpret_cast<Dword *>(assignment[3]));
            break;
        default:
            break;
        }
    }
}
} // namespace

void pass_constants(Dword *pass, Dword state_address) {
    constants(pass, state_address, true);
}

void technique_constants(Dword *technique, Dword state_address) {
    constants(technique, state_address, false);
}

void begin(Dword *dictionary, Dword *pass_count, Dword state_address) {
    auto *selected = reinterpret_cast<Dword *>(dictionary[5]);
    *pass_count = reinterpret_cast<Dword *>(selected[dictionary[8]])[2];

    selected = reinterpret_cast<Dword *>(dictionary[5]);
    technique_constants(reinterpret_cast<Dword *>(selected[dictionary[8]]), state_address);
}

void end(Dword *dictionary, Dword pass_index, Dword state_address) {
    auto *selected = reinterpret_cast<Dword *>(dictionary[5]);
    auto *technique = reinterpret_cast<Dword *>(selected[dictionary[8]]);
    auto *passes = reinterpret_cast<Dword *>(technique[3]);
    pass_constants(reinterpret_cast<Dword *>(passes[pass_index]), state_address);
}
 
void finish(Dword *) {
}
} // namespace nfs::graphics::techniques
