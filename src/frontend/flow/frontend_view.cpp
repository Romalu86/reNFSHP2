#include "nfs/core/application/application.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::application::pending {

void create_frontend_view(void *frontend, Byte frontend_view) {
    using namespace gui::engine;
    gui::String filename;
    gui::string_construct(&filename);
    alignas(4) Byte configuration[56];
    try {
        gui::string_assign(&filename, "SYSTEM\\GDC.INI");
        gui::engine::pending::create_configuration(configuration, &filename);
    } catch (...) {
        gui::string_destruct(&filename);
        throw;
    }
    gui::string_destruct(&filename);
    try {
        gui::engine::pending::select_section(configuration, "VIEWSPACE");
        auto *viewspace = reinterpret_cast<Rectangle *>(game::runtime::bytes(frontend) + 84);
        gui::engine::pending::read_rectangle(configuration, "PC", viewspace, &default_bounds);
        float width = 640, height = 480;
        if (graphics::context) {
            graphics::backend::get_dimensions(graphics::context, &width, &height);
            const auto format = kernel::numeric::current_format();
            const int pixel_height =
                int(kernel::numeric::Extended(height, format).truncate_to_integer());
            const int pixel_width =
                int(kernel::numeric::Extended(width, format).truncate_to_integer());
            Rectangle pixels{rectangle_methods, 0, 0, pixel_width, pixel_height};
            configure_coordinates(viewspace, &pixels);
            input::mouse_bounds(0, 0, 0, pixel_width, pixel_height, 255);
        }
        fonts::faces::begin_font_pool();
        auto *view = static_cast<Engine *>(gui::allocate(sizeof(Engine)));
        if (view) {
            try {
                construct(view, graphics::context);
            } catch (...) {
                gui::release(view);
                throw;
            }
        }
        game::runtime::at<Engine *>(frontend, frontend_view ? 4 : 8) = view;
        fonts::faces::end_font_pool();
        if (view) {
            game::runtime::invoke(view, 244 / 4);
            game::runtime::invoke(view, 292 / 4);
        }
    } catch (...) {
        gui::engine::pending::destroy_configuration(configuration);
        throw;
    }
    gui::engine::pending::destroy_configuration(configuration);
}
} // namespace nfs::application::pending
