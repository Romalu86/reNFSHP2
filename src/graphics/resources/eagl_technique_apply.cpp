#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::graphics::techniques {

void pass_apply(Dword *pass) {
    Dword state[17];
    lifecycle::construction::render_state(state);
    for (int index = 0; index < static_cast<int>(pass[0]); ++index) {
        auto *assignments = reinterpret_cast<Dword *>(pass[1]);
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
            backend::apply_texture_delta(
                reinterpret_cast<backend::TextureStageState *>(assignment[0]));
            break;
        case 5:
            *reinterpret_cast<Dword *>(reinterpret_cast<Dword>(state) + assignment[0]) =
                assignment[3];
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
    geoprim::apply(state);
    Dword validation_texture[12];
    tar::construct_shape(
        validation_texture,
        reinterpret_cast<Dword *>(tar::validation_image +
                                 reinterpret_cast<Dword *>(tar::validation_image)[5]));
    for (Dword stage = 0; stage < backend::adapter_caps()[700 / 4] && static_cast<int>(stage) < 8;
         ++stage) {

        tar::set_stage(validation_texture + 10, stage);
        tar::bind(validation_texture);
    }
    tar::destroy(validation_texture);
    geoprim::base_destroy(state);
}
} // namespace nfs::graphics::techniques
