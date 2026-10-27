#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/backend/eagl_state.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/localization/region.hpp"
#include "nfs/core/platform/cpu.hpp"
#include "nfs/core/timing/timer.hpp"
#include <intrin.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
namespace nfs::game::runtime::particles::pending {
kernel::Dword *find_fsh_image(kernel::Dword *, const char *);
}

namespace nfs::services {


char *register_render_parameter(const char *name, void *value) {
    return eagl::pool_register(&eagl::external_symbols, name, reinterpret_cast<Dword>(value));
}

void initialize_shadow(void *object) {
    register_render_parameter("gShadowColor", static_cast<Byte *>(object) + 80);
    register_render_parameter("gShadowTransform", static_cast<Byte *>(object) + 16);
    register_render_parameter("gAmbientShadow", pending::ambient_shadow);
    auto *state = reinterpret_cast<Dword *>(static_cast<Byte *>(object) + 112);
    graphics::geoprim::shading(state, 1);
    graphics::geoprim::texture_enable(state, 1);
    graphics::geoprim::alpha_test(state, 0);
    graphics::geoprim::alpha_blend_mode(state, 1);
    graphics::geoprim::transparency(state, 1);
    graphics::geoprim::culling(state, 1);
    graphics::geoprim::depth_method(state, 4);
}

void load_shadow_level(void *object) {
    if (!graphics::device)
        return;
    auto *vehicles = lifecycle::manager->slots[52 / 4];
    auto *image = field<Dword *>(vehicles, 78600);
    Dword *shape = game::runtime::particles::pending::find_fsh_image(image, "spla");
    auto *storage =
        static_cast<Dword *>(lifecycle::allocate(48, "Splat TAR", lifecycle::track_pool));
    if (storage) {
        try {
            graphics::tar::construct_shape(storage, shape);
        } catch (...) {
            gui::application_release(storage);
            throw;
        }
    }
    field<Dword *>(object, 108) = storage;
    auto *rules = reinterpret_cast<Dword *>(lifecycle::manager->slots[72 / 4]);
    Dword reverse = 1;
    if (network::packets::rule_value(rules, 6) == 2 || network::packets::rule_value(rules, 6) == 3)
        reverse = 0;
    graphics::geoprim::cull_direction(reinterpret_cast<Dword *>(static_cast<Byte *>(object) + 112),
                                      reverse);
}

void unload_shadow_level(void *object) {
    if (auto *shape = field<Dword *>(object, 108)) {
        graphics::tar::destroy(shape);
        eagl::deallocator(shape, 48);
        field<Dword *>(object, 108) = nullptr;
    }
}

void initialize_game_rules(void *object) {
    field<Dword>(object, 103012) = field<Dword>(object, 103008) = 1;
    field<Dword>(object, 176) = 1;
    field<Dword>(object, 184) = 0;
    field<Dword>(object, 180) =
        pending::opponents_override ? std::atoi(pending::opponents_text) : 0;
    field<Dword>(object, 168) = field<Dword>(object, 180) + 1;
    field<Dword>(object, 188) = 42;
    field<Dword>(object, 192) = field<Dword>(object, 140) = 0;
    field<Dword>(object, 144) = static_cast<Dword>(__rdtsc());
    field<Dword>(object, 148) = 1;
    field<Dword>(object, 152) = field<Dword>(object, 220) = field<Dword>(object, 224) = 0;
    field<Dword>(object, 228) = 2;
    field<Dword>(object, 232) = 1;
    field<Dword>(object, 236) = field<Dword>(object, 240) = 0;
    field<Dword>(object, 244) = 1;
    for (unsigned offset : {172u, 200u, 184u, 204u})
        field<Dword>(object, offset) = 0;
    field<Dword>(object, 208) = field<Dword>(object, 2360) = 1;
    const int language = region::get();
    field<int>(object, 2312) = language;
    field<Dword>(object, 2364) = field<Dword>(object, 2368) = field<Dword>(object, 2372) = 0;
    field<Dword>(object, 2376) = 1;
    if (language == 2 || language == 1 || language == 5 || language == 4 || language == 3 ||
        language == 6)
        field<Dword>(object, 2364) = 1;
    field<Dword>(object, 316) = field<Dword>(object, 248) = 1;
    field<Dword>(object, 252) = 0;
    field<Dword>(object, 308) =
        pending::car_override
            ? find_catalog_car(
                  static_cast<Byte *>(static_cast<void *>(lifecycle::manager->slots[52 / 4])) + 4,
                  pending::car_name)
            : 15;
    for (unsigned index = 0; index < 23; ++index) {
        const unsigned offset = 332 + index * 84;
        field<Dword>(object, offset + 68) = 1;
        field<Dword>(object, offset) = field<Dword>(object, offset + 4) = 0;
    }
    auto *caps = graphics::startup::display_caps;
    if (kernel::cpu::initialize() < 800000000 ||
        !graphics::textures::supports_dxt3(&graphics::textures::database) ||
        (caps && (caps[516 / 4] == 268587332 || caps[516 / 4] == 268587353))) {
        field<Dword>(object, 2380) = 1;
        field<Dword>(object, 2384) = 0;
        field<Dword>(object, 2388) = 1;
    } else if (kernel::cpu::initialize() < 1200000000 || !graphics::state::supports_hardware_vertex_programs()) {
        field<Dword>(object, 2380) = 2;
        field<Dword>(object, 2384) = 3;
        field<Dword>(object, 2388) = 3;
    } else {
        field<Dword>(object, 2380) = 2;
        field<Dword>(object, 2384) = 4;
        field<Dword>(object, 2388) =
            caps && std::strstr(reinterpret_cast<char *>(caps) + 4, "RADEON") &&
                    std::strstr(reinterpret_cast<char *>(caps) + 4, "8500")
                ? 3
                : 4;
    }
    field<Dword>(object, 2320) = 0;
    field<Dword>(object, 2392) = field<Dword>(object, 2396) = 1;
    field<Dword>(object, 2316) = 3;
    field<Dword>(object, 2324) = 4;
}


void initialize_frontend(void *object) {
    pending::initialize_frontend_services();
    char filename[260];
    const auto *paths = reinterpret_cast<char *>(lifecycle::manager->slots[1]);
    std::sprintf(filename, "%sGUI.viv", paths + 2604);
    void *archive = files::file::try_load(filename, lifecycle::ui_pool);
    field<void *>(object, 80) = archive;
    set_gui_archive(archive);
    field<Dword>(object, 52) = lifecycle::ui_pool;

    lifecycle::ui_pool = lifecycle::track_pool;
    pending::load_frontend_screens(object, 1);
    const int frequency = static_cast<int>(kernel::timer_frequency);
    set_gui_repeat_timing(static_cast<Dword>(frequency / 4),
                          static_cast<Dword>(static_cast<int>(Dword(frequency) * 8) / 10));
}

} // namespace nfs::services
