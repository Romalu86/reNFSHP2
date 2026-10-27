#include "nfs/graphics/display/graphics_error.hpp"
#include "nfs/audio/music/music_clients.hpp"
namespace nfs::graphics::pending {
kernel::Dword &audio_focus_disabled =
    *reinterpret_cast<kernel::Dword *>(game::audio::state::client_configuration + 16);
}
