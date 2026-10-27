#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/graphics/display/graphics_error.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/network/session/network_rules.hpp"
namespace nfs::launch {
Options options{};
}
namespace nfs::application::pending {
kernel::Dword &fast_title_enabled =
    *reinterpret_cast<kernel::Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 128);
} // namespace nfs::application::pending
namespace nfs::graphics::startup {
Dword &configuration_variant_enabled =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 132);
char (&configuration_variant)[24] =
    *reinterpret_cast<char (*)[24]>(reinterpret_cast<unsigned char *>(&launch::options) + 1100);
Dword &fullscreen_override =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 60);
Dword &windowed_override =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 104);
Dword &refresh_context =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 148);
} // namespace nfs::graphics::startup
namespace nfs::game::audio::state {
Dword &disable_audio =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 8);
Dword &disable_music =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 24);
Dword &quick_fade =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 128);
} // namespace nfs::game::audio::state
namespace nfs::lifecycle {
Dword &dedicated =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 92);
}
namespace nfs::lifecycle::construction::objects::pending {
Dword &race_seed_enabled =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 248);
char (&race_seed_text)[24] =
    *reinterpret_cast<char (*)[24]>(reinterpret_cast<unsigned char *>(&launch::options) + 1796);
} // namespace nfs::lifecycle::construction::objects::pending
namespace nfs::lifecycle::construction::unresolved {
Dword graphics_screen_resolution_setting = 4294967295u; // Initialized scalar setting.
Dword graphics_colour_depth_setting{}, graphics_gamma_setting{}, reflection_and_driver_stats_shared_setting{};
} // namespace nfs::lifecycle::construction::unresolved
namespace nfs::game::runtime::pending {
Dword &particles_disabled =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 160);
}
namespace nfs::game::runtime::physics::pending {
Dword &collision_disabled =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 40);
}
namespace nfs::network::rules::pending {
Dword &narrow_language_only =
    *reinterpret_cast<Dword *>(reinterpret_cast<unsigned char *>(&launch::options) + 68);
}
