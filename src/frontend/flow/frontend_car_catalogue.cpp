#include "../screens/frontend_menu_roots.hpp"
#include <cstring>
namespace nfs::application::frontend {
using game::runtime::at;
using game::runtime::bytes;

std::uint8_t catalogue_entry_enabled(void *catalogue, int row) {
    return at<std::uint8_t>(catalogue, Dword(row) * 1228 + 28);
}

std::uint8_t catalogue_entry_traffic(void *catalogue, int row) {
    return at<std::uint8_t>(catalogue, Dword(row) * 1228 + 36);
}

const char *catalogue_entry_resource(void *catalogue, int row) {
    return reinterpret_cast<char *>(bytes(catalogue) + Dword(row) * 1228 + 8);
}

Dword catalogue_resource_make(void *catalogue, const char *name) {
    for (int i = 0; i < at<int>(catalogue, 0); ++i)
        if (!_stricmp(name, catalogue_entry_resource(catalogue, i)))
            return at<Dword>(catalogue, Dword(i) * 1228 + 20);
    return Dword(-1);
}

Dword catalogue_resource_name(void *catalogue, const char *name) {
    for (int i = 0; i < at<int>(catalogue, 0); ++i)
        if (!_stricmp(name, catalogue_entry_resource(catalogue, i)))
            return at<Dword>(catalogue, Dword(i) * 1228 + 24);
    return Dword(-1);
}

Dword catalogue_specification(void *catalogue, int row, Dword kind) {
    return kind < 4 ? at<Dword>(catalogue, Dword(row) * 1228 + 48 + kind * 4) : 0;
}
} // namespace nfs::application::frontend
