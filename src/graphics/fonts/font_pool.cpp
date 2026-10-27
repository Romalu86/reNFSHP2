#include "nfs/graphics/fonts/font_faces.hpp"
namespace nfs::fonts::faces {
kernel::Dword saved_pool = 0;

void begin_font_pool() {
    saved_pool = lifecycle::current_pool;
    lifecycle::current_pool = gui::application_pool | 256;
}

void end_font_pool() {
    lifecycle::current_pool = saved_pool;
}
} // namespace nfs::fonts::faces
