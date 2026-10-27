#pragma once
#include "../../math/precision/extended_float.hpp"
namespace nfs::gui::engine {
struct Engine;
struct FontStyle;
kernel::numeric::Extended retained_font_height(int, float);
void font_ascent_native80(int, float, void *);
void initialize_font_metrics(Engine *, FontStyle *);
void normalize_font_scale(FontStyle *, float);
} // namespace nfs::gui::engine
