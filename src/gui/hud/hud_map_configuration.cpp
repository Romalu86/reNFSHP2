#include "hud_map.hpp"
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
static const gui::engine::pending::FlagDefinition map_styles[] = {{"HMS_TRACKING", 0, 1},
                                                                  {nullptr, 0, 0}};

Dword __fastcall load_map(Map *map, void *, void *configuration) {
    using namespace gui::engine::pending;
    read_flags(configuration, "HudMap.mHMStyle", &map->style, map_styles, 0);
    read_image_resource(configuration, "HudMap.mCarShape", &map->car_shape);
    read_image_resource(configuration, "HudMap.mOppShape", &map->opponent_shape);
    read_image_resource(configuration, "HudMap.mPerpShape", &map->perpetrator_shape);
    read_image_resource(configuration, "HudMap.mCopShape", &map->cop_shape);
    const Dword fallback = 0;
    read_colour(configuration, "HudMap.mBackground", &map->background, &fallback);
    read_colour(configuration, "HudMap.mSpline", &map->spline_colour, &fallback);
    read_float(configuration, "HudMap.mZoom", &map->zoom, map->zoom);
    return gui::widgets::pending::load_general(&map->base, nullptr, configuration);
}

Dword __fastcall save_map(Map *map, void *, void *configuration) {
    using namespace gui::engine::pending;
    write_flags(configuration, "HudMap.mHMStyle", map->style, map_styles, 0);
    write_image_resource(configuration, "HudMap.mCarShape", &map->car_shape);
    write_image_resource(configuration, "HudMap.mOppShape", &map->opponent_shape);
    write_image_resource(configuration, "HudMap.mPerpShape", &map->perpetrator_shape);
    write_image_resource(configuration, "HudMap.mCopShape", &map->cop_shape);
    const Dword fallback = 0;
    write_colour(configuration, "HudMap.mBackground", &map->background, &fallback);
    write_colour(configuration, "HudMap.mSpline", &map->spline_colour, &fallback);
    write_float(configuration, "HudMap.mZoom", map->zoom, 2, 0);
    return gui::widgets::pending::save_general(&map->base, nullptr, configuration);
}
} // namespace nfs::application::frontend::hud
