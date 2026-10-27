#include "nfs/graphics/configuration/display_configuration.hpp"
namespace nfs::graphics::symbols {
Dword ambient_colour[4]{1065353216, 1065353216, 1065353216, 1065353216};
Dword light_direction[4]{0, 0, 0, 1065353216};
Dword light_colour[4]{0, 0, 0, 1065353216};
Dword shadow_colour_scale[4]{1065353216, 1065353216, 1065353216, 1065353216};
namespace {
template <unsigned N> bool equal_bytes(const char *name, const char (&expected)[N]) {
    for (unsigned i = 0; i != N; ++i)
        if (name[i] != expected[i])
            return false;
    return true;
}
} // namespace

Dword resolve_lighting(const char *name, std::uint8_t *found) {
    *found = 1;
    if (equal_bytes(name, "gpBoomAmbientColor"))
        return reinterpret_cast<Dword>(ambient_colour);
    if (equal_bytes(name, "gpBoomLightDir"))
        return reinterpret_cast<Dword>(light_direction);
    if (equal_bytes(name, "gpBoomLightColor"))
        return reinterpret_cast<Dword>(light_colour);
    *found = 0;
    return 0;
}
// The 24-byte comparison includes two trailing zero bytes.
Dword resolve_module(const char *name, std::uint8_t *found) {
    constexpr char shadow_name[24] = "gStaticShadowColorScale";
    *found = 1;
    if (equal_bytes(name, shadow_name))
        return reinterpret_cast<Dword>(shadow_colour_scale);
    Dword result = resolve_lighting(name, found);
    if (!*found) {
        *found = 0;
        result = 0;
    }
    return result;
}
} // namespace nfs::graphics::symbols
