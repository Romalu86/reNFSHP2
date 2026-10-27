#include "gui_widget_image_list.hpp"
#include "gui_widget_size.hpp"
#include "gui_widget_family_style.hpp"
#include "../lifecycle/gui_configuration_values.hpp"
namespace nfs::gui::widgets::family {
using namespace engine::pending;

Dword __fastcall image_list_load(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    auto *list = image_list(widget);
    read_size(configuration, "GImageList.mCellSize", &list->cell_size, &default_size);
    read_size(configuration, "GImageList.mImageSize", &list->image_size, &default_size);
    read_integer(configuration, "GImageList.mCenterCell", &list->center_cell, 0);
    read_float(configuration, "GImageList.mSpeed", &list->speed, 0);
    read_image_sequence(configuration, "GImageList.mAnim", &list->sequence);
    read_flags(configuration, "GImageList.mILStyle", &list->style, ils_clip_flags, 0);
    refresh_image_frames(widget);
    return pending::load_general(&list->base, nullptr, configuration);
}
} // namespace nfs::gui::widgets::family
