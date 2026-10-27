#include "track_render_resources.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstring>
namespace nfs::game::world::track_render {
// The unused SymbolView::defined/padding word is not initialized on failure.
eagl::SymbolView *first_model(eagl::SymbolView *result, eagl::Loader *loader) {
    int cursor = 0;
    eagl::SymbolView view;
    view.name = nullptr;
    view.address = 0;
    if (eagl::next_symbol(loader, "Model", &cursor, &view))
        std::memcpy(result, &view, sizeof(view));
    else {
        result->name = nullptr;
        result->type = nullptr;
        result->address = 0;
        std::memcpy(reinterpret_cast<Byte *>(result) + 12,
                    reinterpret_cast<const Byte *>(&view) + 12, 4);
    }
    return result;
}

eagl::SymbolView *find_model(eagl::SymbolView *result, eagl::Loader *loader, const char *name,
                             Byte exact) {
    int cursor = 0;
    eagl::SymbolView view;
    view.name = nullptr;
    view.address = 0;
    if (graphics::device)
        while (eagl::next_symbol(loader, "Model", &cursor, &view)) {
            if (exact ? std::strcmp(view.name, name) == 0
                      : std::strstr(view.name, name) != nullptr) {
                std::memcpy(result, &view, sizeof(view));
                return result;
            }
        }
    result->name = nullptr;
    result->type = nullptr;
    result->address = 0;
    std::memcpy(reinterpret_cast<Byte *>(result) + 12, reinterpret_cast<const Byte *>(&view) + 12,
                4);
    return result;
}

Dword *first_model_property(void *model, const char *prefix) {
    Dword cursor = at<Dword>(model, 0);
    while (cursor != at<Dword>(model, 0) + at<Dword>(model, 4) * 16) {
        if (!std::strncmp(prefix, at<const char *>(reinterpret_cast<void *>(cursor), 0),
                          std::strlen(prefix)))
            break;
        cursor += 16;
    }
    return reinterpret_cast<Dword *>(cursor);
}

Dword *last_model_property(void *model, const char *prefix) {
    Dword first = at<Dword>(model, 0), cursor = first + at<Dword>(model, 4) * 16 - 16;
    while (cursor != first - 16) {
        if (!std::strncmp(prefix, at<const char *>(reinterpret_cast<void *>(cursor), 0),
                          std::strlen(prefix)))
            return reinterpret_cast<Dword *>(cursor + 16);
        cursor -= 16;
    }
    return reinterpret_cast<Dword *>(at<Dword>(model, 0) + at<Dword>(model, 4) * 16);
}

Dword *find_render_layer(void *model, const char *name) {
    const char **names = at<const char **>(model, 160);
    for (int index = 0; index < at<int>(model, 156); ++index)
        if (!std::strcmp(name, names[index]))
            return at<Dword *>(model, 196) + 1 + index;
    return nullptr;
}

void enable_mirror_indices(void *model) {
    Dword *entry = first_model_property(model, "ComputationIndex::MirrorIdx");
    Dword *end = last_model_property(model, "ComputationIndex::MirrorIdx");
    for (; entry != end; entry += 4)
        if (int(entry[2]) > 0)
            *reinterpret_cast<std::uint16_t *>(entry[3]) = 1;
}
namespace {
void set_states(void *model, const char *prefix, Dword depth, bool include_shading) {
    Dword *entry = first_model_property(model, prefix);
    Dword *end = last_model_property(model, prefix);
    for (; entry != end; entry += 4) {
        auto *state = reinterpret_cast<Dword *>(entry[3]);
        if (include_shading) {
            graphics::geoprim::shading(state, 1);
            graphics::geoprim::texture_enable(state, 1);
        }
        graphics::geoprim::alpha_blend_mode(state, 1);
        graphics::geoprim::transparency(state, 1);
        graphics::geoprim::depth_method(state, depth);
        graphics::geoprim::alpha_reference(state, 0);
        graphics::geoprim::alpha_method(state, 5);
        graphics::geoprim::alpha_test(state, 1);
    }
}
bool mirrored_race() {
    if (network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 6) == 2)
        return true;
    return network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 6) == 3;
}
void bind_layers(void *owner, unsigned start, void *model) {
    constexpr const char *names[] = {"opaque",    "alpha1",    "alpha2",    "alpha3",
                                     "alpha4",    "alpha5",    "opaqueLOD", "alpha1LOD",
                                     "alpha2LOD", "alpha3LOD", "alpha4LOD", "alpha5LOD"};
    for (unsigned layer = 0; layer < 12; ++layer)
        at<Dword *>(owner, start + layer * 4) = find_render_layer(model, names[layer]);
}
} // namespace

void configure_track_states(void *model) {
    set_states(model, "GifState::TrackState", 7, true);
    set_states(model, "GifState::Pass2State", 4, true);
}

void configure_boom_states(void *model) {
    if (mirrored_race())
        enable_mirror_indices(model);
    set_states(model, "GifState", 7, false);
}

void set_alpha_references(void *compartment) {
    void *model = at<void *>(compartment, 80);
    Dword *end = last_model_property(model, "State");
    for (Dword *entry = first_model_property(model, "State"); entry != end; entry += 4) {
        const Dword state = entry[3];
        if (!state)
            continue;
        const char *name = reinterpret_cast<const char *>(entry[0]);
        if (!std::strncmp(name, "State::alpha", 12)) {
            Dword reference;
            switch (name[12]) {
            case '1':
                reference = 128;
                break;
            case '2':
                reference = 170;
                break;
            case '3':
                reference = 200;
                break;
            case '4':
                reference = 250;
                break;
            case '5':
                reference = 254;
                break;
            default:
                continue;
            }
            graphics::geoprim::alpha_reference(reinterpret_cast<Dword *>(state), reference);
        }
    }
}

void bind_compartment_model(void *owner, eagl::Loader *loader) {
    eagl::SymbolView view;
    void *model = reinterpret_cast<void *>(first_model(&view, loader)->address);
    at<void *>(owner, 80) = model;
    configure_track_states(model);
    if (mirrored_race())
        enable_mirror_indices(model);
    set_alpha_references(owner);
    bind_layers(owner, 84, at<void *>(owner, 80));
}

void bind_directory_model(void *owner, eagl::Loader *loader) {
    eagl::SymbolView view;
    void *model = reinterpret_cast<void *>(first_model(&view, loader)->address);
    at<void *>(owner, 28) = model;
    if (mirrored_race())
        enable_mirror_indices(model);
    configure_track_states(model);
    bind_layers(owner, 32, model);
}
} // namespace nfs::game::world::track_render
