#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_network_pages.hpp"
#include "nfs/network/session/network_construction.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using network::players::Visitor;
Visitor statistics_visitor{};
extern const Dword statistics_visitor_methods[2];

static void destroy_statistics_visitor(Visitor *visitor) {
    visitor->methods = lifecycle::construction::network::pending::visitor_interface_methods;
}

static Visitor *construct_statistics_visitor(Visitor *visitor) {
    visitor->methods = lifecycle::construction::network::pending::visitor_interface_methods;
    visitor->methods = statistics_visitor_methods;
    return visitor;
}

static Visitor *__fastcall delete_statistics_visitor(Visitor *visitor, void *, Dword flags) {
    destroy_statistics_visitor(visitor);
    if (flags & 1)
        gui::application_release(visitor);
    return visitor;
}
const Dword statistics_visitor_methods[2] = {
    reinterpret_cast<Dword>(visit_network_statistics),
    reinterpret_cast<Dword>(delete_statistics_visitor)};

static void finalize_statistics_visitor() {
    destroy_statistics_visitor(&statistics_visitor);
}

void initialize_statistics_visitor() {
    construct_statistics_visitor(&statistics_visitor);
    std::atexit(finalize_statistics_visitor);
}
} // namespace nfs::application::frontend
NFS_KERNEL_INITIALIZER(startup_initialize_statistics_visitor, ".CRT$XCU_NFS_0069026C",
                       nfs::application::frontend::initialize_statistics_visitor);
