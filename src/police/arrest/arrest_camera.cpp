#include "arrest_cinematic_runtime.hpp"
#include "../../frontend/flow/frontend_runtime.hpp"
namespace nfs::application::pending {
void *find_child(void *, const char *);
void *create_widget(const gui::String *, const gui::String *, const gui::String *, void *);
void set_widget_visible(void *, Dword);
void bring_widget_to_front(void *);

void set_letterbox_visible(Dword visible, void *parent) {
    if (!parent)
        return;
    void *widget = find_child(parent, "<LETTERBOX>");
    if (!widget) {
        LocalFrontendString type("HP2LetterBox"), name("<LETTERBOX>"),
            resource("System\\GInterface.gui");
        widget = create_widget(&resource.value, &name.value, &type.value, parent);
        if (!widget)
            return;
    }
    set_widget_visible(widget, visible);
    bring_widget_to_front(widget);
}
} // namespace nfs::application::pending
namespace nfs::game::world::actor_resources {

void *find_actor_by_name(void *list, const char *name) {
    for (void *node = at<void *>(list, 4); node; node = at<void *>(node, 12))
        if (!std::strcmp(reinterpret_cast<const char *>(bytes(at<void *>(node, 368)) + 40),
                         name))
            return bytes(node) - 16;
    return nullptr;
}

void *find_system_actor(void *owner, const char *name) {
    return at<void *>(owner, 92) ? find_actor_by_name(at<void *>(owner, 92), name) : nullptr;
}

void unload_renderable_driver(void *renderable) {
    vehicle_render::unload_driver(offset(renderable, 856));
}

void unload_vehicle_drivers(void *owner) {
    const int count = at<int>(owner, 81304);
    void *renderable = at<void *>(owner, 81268);
    for (int index = 0; index < count; ++index)
        unload_renderable_driver(bytes(renderable) + Dword(index) * 4064);
}
} // namespace nfs::game::world::actor_resources
namespace nfs::game::world::camera {

void begin_arrest_camera(void *director) {
    application::pending::set_letterbox_visible(1, at<void *>(subsystem(96), 40));
    if (void *actor = actor_resources::find_system_actor(subsystem(48), "camera")) {
        void *controller = find_camera_mode(director, 6);
        if (void *transform = at<void *>(controller, 92))
            at<void *>(transform, 4) = actor;
        if (void *clip = at<void *>(controller, 100))
            at<void *>(clip, 4) = actor;
    }
    if (at<Dword>(director, 356))
        end_camera_mode(director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4));
    if (at<int>(director, 356) < 10)
        ++at<Dword>(director, 356);
    at<Dword>(director, 312 + at<Dword>(director, 356) * 4) = 6;
    begin_camera_mode(director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4));
    notify_camera_mode(director);
    invoke(director, 1);
}
// Proven complete RET leaf.
void end_arrest_camera(void *) {
}
} // namespace nfs::game::world::camera
