#include "movie_runtime.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
namespace nfs::movies {

float *assign_vector(float *output, float x, float y, float z, float w) {
    output[0] = x;
    output[1] = y;
    output[2] = z;
    output[3] = w;
    return output;
}

void draw_image(Image *image, Dword *texture) {
    const float x = image->x, y = image->y;
    const float right = float(number(image->scale_x) * image->width + x);
    const float bottom = float(number(image->scale_y) * image->height + y);
    const float u = float(number(image->width) / integer(field<std::int16_t>(image->shape, 4)));
    const float v = float(number(image->height) / integer(field<std::int16_t>(image->shape, 6)));
    graphics::immediate::set_texture(image->immediate, reinterpret_cast<Dword>(texture));
    graphics::immediate::set_textured_primitive(image->immediate, 5);
    float coordinates[4], position[4];
    Dword colour = 4294967295u;
    assign_vector(coordinates, 0, 0, 1, 1);
    assign_vector(position, x, y, image->z, 1);
    graphics::immediate::textured_vertex(image->immediate, position, &colour, coordinates);
    assign_vector(coordinates, u, 0, 1, 1);
    colour = 4294967295u;
    assign_vector(position, right, y, image->z, 1);
    graphics::immediate::textured_vertex(image->immediate, position, &colour, coordinates);
    assign_vector(coordinates, 0, v, 1, 1);
    colour = 4294967295u;
    assign_vector(position, x, bottom, image->z, 1);
    graphics::immediate::textured_vertex(image->immediate, position, &colour, coordinates);
    assign_vector(coordinates, u, v, 1, 1);
    colour = 4294967295u;
    assign_vector(position, right, bottom, image->z, 1);
    graphics::immediate::textured_vertex(image->immediate, position, &colour, coordinates);
    graphics::immediate::end_textured_primitive(image->immediate);
    graphics::immediate::flush_textured(image->immediate);
}
} // namespace nfs::movies
