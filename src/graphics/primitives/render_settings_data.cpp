#include "nfs/graphics/configuration/render_settings.hpp"
namespace nfs::graphics::startup {
RenderSettings settings = {};
void *settings_owner = nullptr;

int width(const RenderSettings *value, std::uint8_t frontend) {
    return frontend ? value->frontend.width : value->race.width;
}

int height(const RenderSettings *value, std::uint8_t frontend) {
    return frontend ? value->frontend.height : value->race.height;
}

bool fullscreen(const RenderSettings *value) {
    return !value->supports_desktop || value->fullscreen != 0;
}
} // namespace nfs::graphics::startup
