#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../profiles/ui/frontend_profile_controls.hpp"
#include "../../profiles/storage/player_profile.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
using ProfileClassKey = const char *(__cdecl *)();
static Widget *profile_child(DriverProfilePage *page, const char *name, ProfileClassKey type) {
    auto *child = static_cast<Widget *>(pending::find_child(page, name));
    return child && invoke<Dword>(child, 2, reinterpret_cast<const void *>(type)) ? child : nullptr;
}

void __fastcall initialize_driver_profile(DriverProfilePage *page, void *) {
    gui::widgets::pending::request_general_layout(&page->base, nullptr);
    page->new_button = profile_child(page, "<NEW>", family::button_class_key);
    page->delete_button = profile_child(page, "<DELETE>", family::button_class_key);
    page->load_button = profile_child(page, "<LOAD>", family::button_class_key);
    page->save_button = profile_child(page, "<SAVE>", family::button_class_key);
    page->preferences = bytes(system(72)) + 2488;
    page->category = reinterpret_cast<ProfileToggle *>(
        profile_child(page, "<DPL>", profiles::profile_list_class_key));
    if (page->category)
        profiles::select_profile_row(page->category, 0);
    page->statistics = reinterpret_cast<family::ListBox *>(
        profile_child(page, "<DSTATS>", family::list_box_class_key));
    page->current_name = profile_child(page, "<CURRP>", family::text_class_key);
    if (page->current_name && page->preferences) {
        gui::String name;
        pending::copy_preference_name(page->preferences, &name);
        try {
            invoke(page->current_name, 7, &name);
        } catch (...) {
            gui::string_destruct(&name);
            throw;
        }
        gui::string_destruct(&name);
    }
    page->events = system(52);
    page->cars = system(40);
    refresh_profile_statistics(page);
}

void __fastcall release_driver_profile(DriverProfilePage *page, void *) {
    if (!page->current_name ||
        gui::string_equal(invoke<const gui::String *>(page->current_name, 8),
                          &lifecycle::construction::unresolved::shared_string))
        create_page_profile(page);
    refresh_profile_statistics(page);
}

void __fastcall driver_profile_child_event(DriverProfilePage *page, void *, Widget *sender,
                                           Dword event, Dword, Dword) {
    if (event == 14) {
        if (sender)
            invoke<Dword>(sender, 2, family::button_class_key);
        return;
    }
    if (event != 43 && event != 70)
        return;
    auto *toggle = sender && invoke<Dword>(sender, 2, profiles::profile_list_class_key)
                       ? reinterpret_cast<ProfileToggle *>(sender)
                       : nullptr;
    if (toggle == page->category) {
        refresh_profile_statistics(page);
        return;
    }
    if (!sender || !invoke<Dword>(sender, 2, family::button_class_key))
        return;
    auto *button = sender;
    if (button == page->new_button)
        create_page_profile(page);
    else if (button == page->delete_button)
        delete_page_profile(page);
    else if (button == page->load_button)
        load_page_profile(page);
    else if (button == page->save_button)
        save_page_profile(page);
}
void *create_driver_profile(void *);
static ClassRecord profile_class;

static void unregister_driver_profile() {
    unregister_class(&profile_class);
}

static void register_driver_profile() {
    register_class(&profile_class, "DriverProfileScreen", create_driver_profile, "GWidget");
    std::atexit(unregister_driver_profile);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_driver_profile, ".CRT$XCU_NFS_00690078",
                       register_driver_profile);
} // namespace
} // namespace nfs::application::frontend
