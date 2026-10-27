#include "gui_widget_family_style.hpp"
#include "gui_widget_resources.hpp"
#include "gui_widget_shared_methods.hpp"
#include "../lifecycle/gui_configuration_values.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets {
Dword read_image_resource(void *, const char *, ImageResource *);
Dword write_image_resource(void *, const char *, ImageResource *);
namespace family {
using game::runtime::at;
using game::runtime::invoke;
using namespace engine::pending;
#include "gui_widget_family_flags.inc"
const char *edit_characters =
#include "gui_edit_characters.inc"
    ;

Dword __fastcall text_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    read_flags(configuration, "GText.mTextStyle", &at<Dword>(widget, 264), text_style_flags, 0);
    read_integer(configuration, "GText.mMargin", &at<int>(widget, 272), 0);
    read_integer(configuration, "GText.mLineSpacing", &at<int>(widget, 276), 0);
    return pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall text_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_flags(configuration, "GText.mTextStyle", at<Dword>(widget, 264), text_style_flags, 0);
    write_int(configuration, "GText.mMargin", at<int>(widget, 272), 0);
    write_int(configuration, "GText.mLineSpacing", at<int>(widget, 276), 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall edit_text_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    read_integer(configuration, "GEditText.mMaxChars", &at<int>(widget, 308), 0);
    read_flags(configuration, "GEditText.mFilter", &at<Dword>(widget, 320), filter_alpha_flags, 0);
    read_flags(configuration, "GEditText.mEditStyle", &at<Dword>(widget, 324),
               ets_forceupper_flags, 0);
    return text_load(widget, nullptr, configuration_address);
}

Dword __fastcall edit_text_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_ulong(configuration, "GEditText.mMaxChars", at<int>(widget, 308), 0);
    write_flags(configuration, "GEditText.mFilter", at<Dword>(widget, 320), filter_alpha_flags, 0);
    write_flags(configuration, "GEditText.mEditStyle", at<Dword>(widget, 324),
                ets_forceupper_flags, 0);
    return text_save(widget, nullptr, configuration_address);
}

Dword __fastcall tool_tip_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_float(configuration, "GToolTip.mDelayTime", at<float>(widget, 272), 2, 0);
    write_float(configuration, "GToolTip.mShowTime", at<float>(widget, 276), 2, 0);
    write_flags(configuration, "GWidget.mWidgetStyle", at<Dword>(widget, 256), general_style_flags,
                0);
    write_flags(configuration, "GWidget.mTabKeys", at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::save_configuration(widget, nullptr, configuration);
}

Dword __fastcall static_image_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    read_image_resource(configuration, "GStaticImage.mShape", invoke<ImageResource *>(widget, 47));
    read_flags(configuration, "GStaticImage.mSIStyle", &at<Dword>(widget, 268),
               static_image_style_flags, 0);
    read_float(configuration, "GStaticImage.mTileX", &at<float>(widget, 316), 1.0f);
    read_float(configuration, "GStaticImage.mTileY", &at<float>(widget, 320), 1.0f);
    read_flags(configuration, "GImage.mImageStyle", &at<Dword>(widget, 264), image_style_flags, 0);
    return pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall static_image_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_image_resource(configuration, "GStaticImage.mShape", invoke<ImageResource *>(widget, 47));
    write_flags(configuration, "GStaticImage.mSIStyle", at<Dword>(widget, 268),
                static_image_style_flags, 0);
    write_float(configuration, "GStaticImage.mTileX", at<float>(widget, 316), 2, 1.0f);
    write_float(configuration, "GStaticImage.mTileY", at<float>(widget, 320), 2, 1.0f);
    write_flags(configuration, "GImage.mImageStyle", at<Dword>(widget, 264), image_style_flags, 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall image_box_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    const Dword zero_colour = 0;
    read_flags(configuration, "GImageBox.mBoxStyle", &at<Dword>(widget, 268),
               image_box_style_flags, 0);
    read_colour(configuration, "GImageBox.mColorUL", &at<Dword>(widget, 272), &zero_colour);
    read_colour(configuration, "GImageBox.mColorUR", &at<Dword>(widget, 276), &zero_colour);
    read_colour(configuration, "GImageBox.mColorBL", &at<Dword>(widget, 280), &zero_colour);
    read_colour(configuration, "GImageBox.mColorBR", &at<Dword>(widget, 284), &zero_colour);
    read_flags(configuration, "GImage.mImageStyle", &at<Dword>(widget, 264), image_style_flags, 0);
    return pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall image_bar_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_flags(configuration, "GImageBar.mIBarStyle", at<Dword>(widget, 268),
                image_bar_style_flags, 0);
    write_flags(configuration, "GImage.mImageStyle", at<Dword>(widget, 264), image_style_flags, 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall button_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_flags(configuration, "GButton.mButtonStyle", at<Dword>(widget, 296), bs_greybutton_flags,
                0);
    write_int(configuration, "GButton.mRadioGroup", at<int>(widget, 304), 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall list_box_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    read_integer(configuration, "GListBox.mRowHeight", &at<int>(widget, 308), 0);
    read_flags(configuration, "GListBox.mListStyle", &at<Dword>(widget, 320), ls_multiselect_flags,
               0);
    read_integer(configuration, "GListBox.mDropRows", &at<int>(widget, 324), 0);
    return pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall list_box_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_int(configuration, "GListBox.mRowHeight", at<int>(widget, 308), 0);
    write_flags(configuration, "GListBox.mListStyle", at<Dword>(widget, 320), ls_multiselect_flags,
                0);
    write_ulong(configuration, "GListBox.mDropRows", at<int>(widget, 324), 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall combo_box_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    read_flags(configuration, "GComboBox.mComboStyle", &at<Dword>(widget, 336),
               cbs_userstrings_flags, 0);
    read_integer(configuration, "GComboBox.mMaxVisible", &at<int>(widget, 332), 0);
    return edit_text_load(widget, nullptr, configuration_address);
}

Dword __fastcall combo_box_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_flags(configuration, "GComboBox.mComboStyle", at<Dword>(widget, 336),
                cbs_userstrings_flags, 0);
    write_int(configuration, "GComboBox.mMaxVisible", at<int>(widget, 332), 0);
    return edit_text_save(widget, nullptr, configuration_address);
}

Dword __fastcall tree_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_int(configuration, "GTree.mIndent", at<int>(widget, 336), 0);
    write_int(configuration, "GTree.mRowHeight", at<int>(widget, 340), 0);
    write_int(configuration, "GTree.mExpandIconSize", at<int>(widget, 352), 0);
    write_flags(configuration, "GTree.mTreeStyle", at<Dword>(widget, 332), trs_lines_flags, 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall frame_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_flags(configuration, "GFrame.mFrameStyle", at<Dword>(widget, 264), fs_greyframe_flags,
                0);
    write_int(configuration, "GFrame.mMinHeight", at<int>(widget, 304), 0);
    write_int(configuration, "GFrame.mMinWidth", at<int>(widget, 308), 0);
    write_int(configuration, "GFrame.mMaxHeight", at<int>(widget, 312), 0);
    write_int(configuration, "GFrame.mMaxWidth", at<int>(widget, 316), 0);
    write_flags(configuration, "GWidget.mWidgetStyle", at<Dword>(widget, 256), general_style_flags,
                0);
    write_flags(configuration, "GWidget.mTabKeys", at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::save_configuration(widget, nullptr, configuration);
}

Dword __fastcall scroll_bar_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    read_flags(configuration, "GScrollBar.mScrollStyle", &at<Dword>(widget, 264), ss_greybar_flags,
               0);
    read_integer(configuration, "GScrollBar.mMinThumbHeight", &at<int>(widget, 332), 10);
    read_flags(configuration, "GWidget.mWidgetStyle", &at<Dword>(widget, 256), general_style_flags,
               0);
    read_flags(configuration, "GWidget.mTabKeys", &at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::load_configuration(widget, nullptr, configuration);
}

Dword __fastcall gripper_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_flags(configuration, "GGripper.mGripperStyle", at<Dword>(widget, 400),
                gripper_style_flags, 0);
    return anim_image_save(widget, nullptr, configuration_address);
}

Dword __fastcall slider_bar_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    read_integer(configuration, "GSliderBar.mMin", &at<int>(widget, 292), 0);
    read_integer(configuration, "GSliderBar.mMax", &at<int>(widget, 296), 0);
    read_integer(configuration, "GSliderBar.mStep", &at<int>(widget, 288), 0);
    read_flags(configuration, "GSliderBar.mSliderStyle", &at<Dword>(widget, 300),
               sbs_greybar_flags, 0);
    read_flags(configuration, "GWidget.mWidgetStyle", &at<Dword>(widget, 256), general_style_flags,
               0);
    read_flags(configuration, "GWidget.mTabKeys", &at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::load_configuration(widget, nullptr, configuration);
}

Dword __fastcall slider_bar_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_int(configuration, "GSliderBar.mMin", at<int>(widget, 292), 0);
    write_int(configuration, "GSliderBar.mMax", at<int>(widget, 296), 0);
    write_int(configuration, "GSliderBar.mStep", at<int>(widget, 288), 0);
    write_flags(configuration, "GSliderBar.mSliderStyle", at<Dword>(widget, 300),
                sbs_greybar_flags, 0);
    write_flags(configuration, "GWidget.mWidgetStyle", at<Dword>(widget, 256), general_style_flags,
                0);
    write_flags(configuration, "GWidget.mTabKeys", at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::save_configuration(widget, nullptr, configuration);
}

Dword __fastcall menu_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    read_flags(configuration, "GMenu.mMenuStyle", &at<Dword>(widget, 276), ms_itemtooltips_flags,
               0);
    read_integer(configuration, "GMenu.mLineSpacing", &at<int>(widget, 264), 0);
    read_integer(configuration, "GMenu.mLeftMargin", &at<int>(widget, 268), 0);
    read_integer(configuration, "GMenu.mRightMargin", &at<int>(widget, 272), 0);
    read_integer(configuration, "GMenu.mMinWidth", &at<int>(widget, 280), 0);
    read_integer(configuration, "GMenu.mOverlap", &at<int>(widget, 284), 0);
    read_image_resource(configuration, "GMenu.mArrow",
                        reinterpret_cast<ImageResource *>(game::runtime::bytes(widget) + 316));
    read_float(configuration, "GMenu.mDelay", &at<float>(widget, 288), 1.0f);
    return pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall menu_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_flags(configuration, "GMenu.mMenuStyle", at<Dword>(widget, 276), ms_itemtooltips_flags,
                0);
    write_int(configuration, "GMenu.mLineSpacing", at<int>(widget, 264), 0);
    write_int(configuration, "GMenu.mLeftMargin", at<int>(widget, 268), 0);
    write_int(configuration, "GMenu.mRightMargin", at<int>(widget, 272), 0);
    write_int(configuration, "GMenu.mMinWidth", at<int>(widget, 280), 0);
    write_int(configuration, "GMenu.mOverlap", at<int>(widget, 284), 0);
    write_image_resource(configuration, "GMenu.mArrow",
                         reinterpret_cast<ImageResource *>(game::runtime::bytes(widget) + 316));
    write_float(configuration, "GMenu.mDelay", at<float>(widget, 288), 2, 1.0f);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall dialog_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    read_flags(configuration, "GDialog.mDialogStyle", &at<Dword>(widget, 272), ds_keyok_flags, 3);
    read_float(configuration, "GDialog.mTimeOut", &at<float>(widget, 280), 0);
    return pending::load_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall dialog_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    write_flags(configuration, "GDialog.mDialogStyle", at<Dword>(widget, 272), ds_keyok_flags, 3);
    write_float(configuration, "GDialog.mTimeOut", at<float>(widget, 280), 2, 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}

Dword __fastcall marquee_load(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    const Dword zero_colour = 0;
    read_colour(configuration, "GMarquee.mHighlightColor", &at<Dword>(widget, 296), &zero_colour);
    read_colour(configuration, "GMarquee.mDisableColor", &at<Dword>(widget, 300), &zero_colour);
    read_flags(configuration, "GMarquee.mMQStyle", &at<Dword>(widget, 292), mqs_move_flags, 0);
    read_flags(configuration, "GWidget.mWidgetStyle", &at<Dword>(widget, 256), general_style_flags,
               0);
    read_flags(configuration, "GWidget.mTabKeys", &at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::load_configuration(widget, nullptr, configuration);
}

Dword __fastcall marquee_save(Widget *widget, void *, Dword configuration_address) {
    void *configuration = reinterpret_cast<void *>(configuration_address);
    const Dword zero_colour = 0;
    write_colour(configuration, "GMarquee.mHighlightColor", &at<Dword>(widget, 296), &zero_colour);
    write_colour(configuration, "GMarquee.mDisableColor", &at<Dword>(widget, 300), &zero_colour);
    write_flags(configuration, "GMarquee.mMQStyle", at<Dword>(widget, 292), mqs_move_flags, 0);
    write_flags(configuration, "GWidget.mWidgetStyle", at<Dword>(widget, 256), general_style_flags,
                0);
    write_flags(configuration, "GWidget.mTabKeys", at<Dword>(widget, 260), tab_key_flags, 31);
    return pending::save_configuration(widget, nullptr, configuration);
}
} // namespace family
} // namespace nfs::gui::widgets
