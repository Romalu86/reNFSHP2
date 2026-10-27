#include "gui_widget_sequence.hpp"
#include "gui_widget_size.hpp"
#include "gui_widget_shared_methods.hpp"
#include "gui_widget_family_style.hpp"
#include "../lifecycle/gui_configuration_values.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using namespace engine::pending;

Dword __fastcall anim_image_load(Widget *widget, void *, Dword address) {
    const Dword result = static_image_load(widget, nullptr, address);
    void *configuration = reinterpret_cast<void *>(address);
    float rate;
    read_float(configuration, "GAnimImage.mFPS", &rate, 0);
    read_image_sequence(configuration, "GAnimImage.mAnim", invoke<ImageSequence *>(widget, 48));
    read_integer(configuration, "GAnimImage.mDefaultFrame", &at<int>(widget, 388), 0);
    read_flags(configuration, "GAnimImage.mAnimStyle", &at<Dword>(widget, 392), ais_noloop_flags,
               0);
    if (rate >= 0) {
        if (const Dword timer = at<Dword>(widget, 396))
            update_widget_timer(timer, rate != 0 ? 1.0f / rate : 0.0f);
        at<float>(widget, 324) = rate;
    }
    select_sequence_frame(reinterpret_cast<ImageSequence *>(bytes(widget) + 328),
                          at<int>(widget, 388));
    return result;
}

Dword __fastcall scroll_view_load(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    read_size(configuration, "GScrollView.mStep", reinterpret_cast<Point *>(bytes(widget) + 284),
              &default_size);
    read_flags(configuration, "GScrollView.mScrollViewStyle", &at<Dword>(widget, 296),
               svs_novscroll_flags, 0);
    read_flags(configuration, "GWidget.mWidgetStyle", &at<Dword>(widget, 256), general_style_flags,
               0);
    read_flags(configuration, "GWidget.mTabKeys", &at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::load_configuration(widget, nullptr, configuration);
}

Dword __fastcall scroll_view_save(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    write_size(configuration, "GScrollView.mStep", reinterpret_cast<Point *>(bytes(widget) + 284),
               &default_size);
    write_flags(configuration, "GScrollView.mScrollViewStyle", at<Dword>(widget, 296),
                svs_novscroll_flags, 0);
    write_flags(configuration, "GWidget.mWidgetStyle", at<Dword>(widget, 256), general_style_flags,
                0);
    write_flags(configuration, "GWidget.mTabKeys", at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::save_configuration(widget, nullptr, configuration);
}

Dword __fastcall image_list_save(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    write_size(configuration, "GImageList.mCellSize",
               reinterpret_cast<Point *>(bytes(widget) + 336), &default_size);
    write_size(configuration, "GImageList.mImageSize",
               reinterpret_cast<Point *>(bytes(widget) + 348), &default_size);
    write_int(configuration, "GImageList.mCenterCell", at<int>(widget, 368), 0);
    write_float(configuration, "GImageList.mSpeed", at<float>(widget, 380), 2, 0);
    write_image_sequence(configuration, "GImageList.mAnim",
                         reinterpret_cast<ImageSequence *>(bytes(widget) + 264));
    write_flags(configuration, "GImageList.mILStyle", at<Dword>(widget, 384), ils_clip_flags, 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall anim_image_save(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    write_float(configuration, "GAnimImage.mFPS", at<float>(widget, 324), 2, 0);
    write_image_sequence(configuration, "GAnimImage.mAnim", invoke<ImageSequence *>(widget, 48));
    write_int(configuration, "GAnimImage.mDefaultFrame", at<int>(widget, 388), 0);
    write_flags(configuration, "GAnimImage.mAnimStyle", at<Dword>(widget, 392), ais_noloop_flags,
                0);
    return static_image_save(widget, nullptr, address);
}

Dword __fastcall gripper_load(Widget *widget, void *, Dword address) {
    read_flags(reinterpret_cast<void *>(address), "GGripper.mGripperStyle",
               &at<Dword>(widget, 400), gripper_style_flags, 0);
    return anim_image_load(widget, nullptr, address);
}

Dword __fastcall image_box_save(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    const Dword style = at<Dword>(widget, 268);
    write_flags(configuration, "GImageBox.mBoxStyle", style, image_box_style_flags, 0);
    if (style & 16) {
        const Dword zero = 0;
        write_colour(configuration, "GImageBox.mColorUL", &at<Dword>(widget, 272), &zero);
        write_colour(configuration, "GImageBox.mColorUR", &at<Dword>(widget, 276), &zero);
        write_colour(configuration, "GImageBox.mColorBL", &at<Dword>(widget, 280), &zero);
        write_colour(configuration, "GImageBox.mColorBR", &at<Dword>(widget, 284), &zero);
    }
    write_flags(configuration, "GImage.mImageStyle", at<Dword>(widget, 264), image_style_flags, 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall image_bar_load(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    read_flags(configuration, "GImageBar.mIBarStyle", &at<Dword>(widget, 268),
               image_bar_style_flags, 0);
    read_flags(configuration, "GImage.mImageStyle", &at<Dword>(widget, 264), image_style_flags, 0);
    return pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall tool_tip_load(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    read_float(configuration, "GToolTip.mDelayTime", &at<float>(widget, 272), 0);
    read_float(configuration, "GToolTip.mShowTime", &at<float>(widget, 276), 0);
    read_flags(configuration, "GWidget.mWidgetStyle", &at<Dword>(widget, 256), general_style_flags,
               0);
    read_flags(configuration, "GWidget.mTabKeys", &at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::load_configuration(widget, nullptr, configuration);
}

Dword __fastcall scroll_bar_save(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    write_flags(configuration, "GScrollBar.mScrollStyle", at<Dword>(widget, 264), ss_greybar_flags,
                0);
    write_int(configuration, "GScrollBar.mMinThumbHeight", at<int>(widget, 332), 10);
    write_flags(configuration, "GWidget.mWidgetStyle", at<Dword>(widget, 256), general_style_flags,
                0);
    write_flags(configuration, "GWidget.mTabKeys", at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::save_configuration(widget, nullptr, configuration);
}

Dword __fastcall frame_load(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    read_flags(configuration, "GFrame.mFrameStyle", &at<Dword>(widget, 264), fs_greyframe_flags,
               0);
    read_integer(configuration, "GFrame.mMinHeight", &at<int>(widget, 304), 0);
    read_integer(configuration, "GFrame.mMinWidth", &at<int>(widget, 308), 0);
    read_integer(configuration, "GFrame.mMaxHeight", &at<int>(widget, 312), 0);
    read_integer(configuration, "GFrame.mMaxWidth", &at<int>(widget, 316), 0);
    if (at<int>(widget, 304) < 1)
        at<int>(widget, 304) = 0;
    if (at<int>(widget, 308) < 1)
        at<int>(widget, 308) = 0;
    read_flags(configuration, "GWidget.mWidgetStyle", &at<Dword>(widget, 256), general_style_flags,
               0);
    read_flags(configuration, "GWidget.mTabKeys", &at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::load_configuration(widget, nullptr, configuration);
}
} // namespace nfs::gui::widgets::family
