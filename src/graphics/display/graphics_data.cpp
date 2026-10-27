#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../race/session/race_deferred_draw.hpp"
#include "nfs/core/configuration/launch_options.hpp"
namespace nfs::graphics {
Device *device = nullptr;
Dword *context = nullptr;
Dword *main_viewport = nullptr;
Dword *ui_viewport = nullptr;
Dword *background_viewport = nullptr;
void *camera = nullptr;
void *module_data = nullptr;
eagl::Loader *module = nullptr;
Dword mode = 0;
std::uint8_t gamma_saved = 0;
WORD gamma[3][256]{};
Dword &server_requested =
    *reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(&launch::options) + 88);
Dword &graphics_disabled =
    *reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(&launch::options) + 92);
void *current_device = nullptr;
Dword *current_context = nullptr;
Dword font_shape_type = 0;
float camera_field_of_view = 0;
} // namespace nfs::graphics
namespace nfs::graphics::unresolved {
// 32-bit scalar storage.
Dword camera_state = 0;
void *&initial_viewport = deferred::queue.viewport;
} // namespace nfs::graphics::unresolved
