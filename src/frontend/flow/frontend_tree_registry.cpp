#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../screens/frontend_menu_roots.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
void *create_tree_screen(void *);
void *create_tree_event_dialog(void *);
static ClassRecord tree_classes[2];

static void unregister_tree_pages() {
    for (unsigned i = 2; i; --i)
        unregister_class(tree_classes + i - 1);
}

static void register_tree_pages() {
    register_class(tree_classes, "TreeScreen", create_tree_screen, "GInterface");
    register_class(tree_classes + 1, "TreeEventDlg", create_tree_event_dialog, "GDialog");
    std::atexit(unregister_tree_pages);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_tree_pages, ".CRT$XCU_NFS_00690160",
                       register_tree_pages);
} // namespace
} // namespace nfs::application::frontend
