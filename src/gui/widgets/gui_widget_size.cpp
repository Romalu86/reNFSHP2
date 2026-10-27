#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "gui_widget_size.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdio>
#include <cstdlib>
namespace nfs::gui::widgets::family {
Point default_size{};

void construct_size(Point *size) {
    *size = Point{vector_methods, 0, 0};
}

static void destroy_default_size() {
    engine::destroy_point(&default_size);
}
 
static void initialize_default_size() {
    construct_size(&default_size);
    std::atexit(destroy_default_size);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_default_size, ".CRT$XCU_NFS_006906C8",
                       initialize_default_size);
} // namespace
// Both the receiver and the hidden result are written.
Point *assign_size(Point *size, Point *out, const Point *source) {
    const int x = source->x, y = source->y;
    size->y = y;
    out->y = y;
    size->x = x;
    out->methods = vector_methods;
    out->x = x;
    return out;
}

static Dword equal_size(const Point *first, const Point *second) {
    return first->x == second->x && first->y == second->y;
}

Dword read_size(void *configuration, const char *key, Point *size, const Point *fallback) {
    const String *value = game::runtime::invoke<const String *>(configuration, 5, key);
    if (value->length)
        return Dword(std::sscanf(string_ascii(value), "[ %d , %d ]", &size->x, &size->y));
    Point temporary;
    assign_size(size, &temporary, fallback);
    return 0;
}
// The serialized type is SIZE, even for a step vector.
Dword write_size(void *configuration, const char *key, const Point *size, const Point *fallback) {
    const Dword same = equal_size(size, fallback);
    char buffer[256]{};
    std::sprintf(buffer, "[%d, %d]", size->x, size->y);
    String text;
    string_construct_ascii(&text, buffer);
    Dword result;
    try {
        result = game::runtime::invoke<Dword>(configuration, 7, key, &text, "SIZE", Dword(0), same);
    } catch (...) {
        string_destruct(&text);
        throw;
    }
    string_destruct(&text);
    return result;
}
} // namespace nfs::gui::widgets::family
