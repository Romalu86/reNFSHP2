#include "nfs/core/application/application.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "../../gui/widgets/gui_widget_family.hpp"
namespace nfs::application::pending {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
void *find_child(void *, const char *);
void set_widget_visible(void *, Dword);
extern Dword &fast_title_enabled;
namespace {
struct ScreenString {
    gui::String value;
    explicit ScreenString(const char *text) {
        gui::string_construct_ascii(&value, text);
    }
    ~ScreenString() {
        gui::string_destruct(&value);
    }
};
void ensure_frontend_view(void *frontend) {
    if (!at<void *>(frontend, 4)) {
        if (void *race_view = at<void *>(frontend, 8)) {
            invoke(race_view, 248 / 4);
            gui::destroy(static_cast<gui::Object *>(race_view));
            at<void *>(frontend, 8) = nullptr;
        }
        const Dword previous_pool = lifecycle::ui_pool;
        lifecycle::ui_pool = lifecycle::track_pool;
        at<Dword>(frontend, 52) = previous_pool;
        create_frontend_view(frontend, 1);
    }
}
void *create_dialog(const char *name) {
    ScreenString type("TitleScreen"), widget_name(name), file("system\\GDialogs.gui");
    void *dialog = create_widget(&file.value, &widget_name.value, &type.value, nullptr);
    return dialog && invoke<Dword>(dialog, 2, gui::widgets::family::dialog_class_key) ? dialog
                                                                                     : nullptr;
}
} // namespace

void __cdecl initialize_title_screen(void *screen) {
    if (gui::context_state) {
        gui::pump_context(gui::context_state);
        if (!gui::poll_context(screen)) {
            auto *root = static_cast<gui::widgets::Widget *>(screen);
            while (root->parent)
                root = root->parent;
            gui::tick_root(root);
        }
    }
    void *owner = system(96);
    void *render = at<void *>(owner, 4) ? at<void *>(owner, 4) : at<void *>(owner, 8);
    at<Byte>(render, 3156) = 0;
    gui::tick_root(screen);
    gui::tick_root(screen);
    for (auto *subsystem : lifecycle::manager->slots)
        if (subsystem)
            subsystem->methods->post_initialize(subsystem);
    ini::flush_cache();
    owner = system(96);
    render = at<void *>(owner, 4) ? at<void *>(owner, 4) : at<void *>(owner, 8);
    at<Byte>(render, 3156) = 1;
}

int run_initial_frontend(void *frontend) {
    ensure_frontend_view(frontend);
    void *dialog = create_dialog(at<Dword>(system(72), 2312) == 1 ? "LoadScreenF" : "LoadScreen");
    if (!dialog)
        return 2;
    if (void *press = find_child(dialog, "<PRESS>"))
        if (invoke<Dword>(press, 2, gui::widgets::family::text_class_key)) {
            gui::widgets::set_enabled(static_cast<gui::widgets::Widget *>(press), 0);
            set_widget_visible(press, 0);
        }
    at<Dword>(dialog, 272) = 0;
    at<float>(dialog, 280) = network::session_active() ? .25f : fast_title_enabled ? .01f : 6.0f;
    gui::run_modal(dialog, initialize_title_screen);
    destroy_screen(dialog);
    return 2;
}

int run_network_frontend(void *frontend) {
    ensure_frontend_view(frontend);
    void *dialog =
        create_dialog(at<Dword>(system(72), 2312) == 1 ? "TitleScreenF" : "TitleScreen");
    int result = 2;
    if (dialog) {
        if (!find_child(dialog, "<CHYRON>")) {
            ScreenString type("ChyronCtl"), name("<CHYRON>"), file("System\\GInterface.gui");
            create_widget(&file.value, &name.value, &type.value, dialog);
        }
        at<Dword>(dialog, 272) = 0;
        at<float>(dialog, 280) = 30.0f;
        result = gui::run_modal(dialog, nullptr);
        destroy_screen(dialog);
    }
    return result;
}
} // namespace nfs::application::pending
