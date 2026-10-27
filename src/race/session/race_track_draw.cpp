#include "race_world_draw.hpp"
#include "race_render_passes.hpp"
#include "../../graphics/cameras/camera_director_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
namespace nfs::game::world::track_render {
static void *player_compartment(void *track, int player, int section) {
    return offset(track, (Dword(section) * 2 + Dword(player) + 1) * 168);
}

void draw_opaque_track(void *track, void *director) {
    ++at<Dword>(track, 12);
    if (at<int>(track, 12) <= 1) {
        void *directory = offset(track, 76);
        prepare_track_directory(directory);
        for (int section = 0; section < at<int>(track, 4); ++section) {
            void *compartment = offset(track, 168 + Dword(section) * 336);
            draw_opaque_layers(compartment, 1);
            draw_surface_layers(compartment, 1);
        }
        draw_track_directory(directory, director);
        return;
    }
    const int player = at<int>(director, 304);
    bool forward = true;
    if (void *car = at<void *>(director, 300)) {
        void *entity = at<void *>(car, 7988);
        if (!at<Byte>(entity, 409)) {
            const auto heading = camera::route_heading_alignment(entity);
            forward = !(heading < kernel::numeric::Extended(0.f, kernel::numeric::current_format()));
        }
    }
    auto *sections = static_cast<Dword *>(track);
    const int first = forward ? runtime::track::next_section(sections, player)
                              : runtime::track::previous_section(sections, player);
    const int second = forward ? runtime::track::previous_section(sections, player)
                               : runtime::track::next_section(sections, player);
    void *directory = offset(track, 76);
    prepare_track_directory(directory);
    const Byte quality = at<int>(subsystem(72), 2380) > 1;
    void *first_compartment = player_compartment(track, player, first);
    draw_opaque_layers(first_compartment, quality);
    draw_opaque_layers(player_compartment(track, player, at<int>(track, 48 + player * 4)), quality);
    void *second_compartment = player_compartment(track, player, second);
    draw_opaque_layers(second_compartment, quality);
    draw_surface_layers(first_compartment, quality);
    draw_surface_layers(player_compartment(track, player, at<int>(track, 48 + player * 4)), quality);
    draw_surface_layers(second_compartment, quality);
    draw_track_directory(directory, director);
    draw_start_line(subsystem(136), director);
}

void draw_reflection_track(void *track, void *director) {
    const int player = at<int>(director, 304);
    void *directory = offset(track, 76);
    prepare_reflection_directory(directory);
    draw_reflection_layers(player_compartment(track, player, at<int>(track, 48 + player * 4)));
    draw_reflection_directory(directory, director);
}

void draw_track_glows(void *track, void *director) {
    const int player = at<int>(director, 304);
    draw_layer_glows(player_compartment(track, player, at<int>(track, 48 + player * 4)));
    const int next = runtime::track::next_section(static_cast<Dword *>(track), player);
    draw_layer_glows(player_compartment(track, player, next));
    const int previous = runtime::track::previous_section(static_cast<Dword *>(track), player);
    draw_layer_glows(player_compartment(track, player, previous));
}

void draw_track_overlay(void *track, void *director) {
    draw_directory_overlay(offset(track, 76), director);
}
// All three stack arguments are ignored by the entry.
void apply_track_fog(void *track, Dword, void *, Dword *) {
    void *section = offset(track, at<Dword>(track, 48) * 336);
    fog::configure(static_cast<fog::State *>(subsystem(128)), at<float>(section, 172),
                     at<float>(section, 176), at<float>(section, 180), at<float>(section, 184),
                     at<Dword>(section, 188), at<Dword>(section, 192), at<int>(section, 220));
}
} // namespace nfs::game::world::track_render
