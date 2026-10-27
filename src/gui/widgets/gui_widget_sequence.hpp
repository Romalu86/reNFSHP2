#pragma once
#include "gui_widget_resources.hpp"
namespace nfs::gui::widgets::family {
ImageSequence *copy_image_sequence(ImageSequence *, const ImageSequence *);
void clear_image_sequence(ImageSequence *);
Dword select_sequence_frame(ImageSequence *, int);
Dword load_image_sequence(ImageSequence *, const String *, int);
String *image_sequence_name(ImageSequence *, String *);
long double sequence_frame_rate(ImageSequence *, Dword);
Dword read_image_sequence(void *, const char *, ImageSequence *);
Dword write_image_sequence(void *, const char *, ImageSequence *);
Dword visible_in_ancestors(Widget *);
Dword inherited_palette(Widget *);
} // namespace nfs::gui::widgets::family
