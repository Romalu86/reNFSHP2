#include "nfs/graphics/fonts/font_renderer.hpp"
namespace nfs::graphics::font_renderer {
const fonts::Callbacks callbacks{draw, begin, end, initialize, release};
const fonts::Settings batch_settings{28, batch, {0, 0, 0, 0, 0}};
} // namespace nfs::graphics::font_renderer
