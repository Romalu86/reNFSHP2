#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
namespace nfs::graphics::immediate {

void set_textured_primitive(Dword *object, Dword primitive) {
    if (primitive != object[0]) {
        flush_textured(object);
        object[0] = primitive;
        geoprim::primitive(object + 1, primitive);
    }
}

void end_textured_primitive(Dword *object) {
    if (object[80 / 4] + 3 < 16385 && object[0] != 5 && object[0] != 3 && object[0] != 6)
        return;
    flush_textured(object);
}

void set_render_state(Dword *object, const Dword *state) {
    geoprim::assign(object + 1, state);
    geoprim::primitive(object + 1, object[0]);
}

void textured_vertex(Dword *object, const float *position, const Dword *colour, const float *uv) {
    const Dword index = object[80 / 4]++;
    auto *vertices = reinterpret_cast<Dword *>(object[76 / 4]);
    auto *vertex = reinterpret_cast<float *>(vertices + index * 6);
    reinterpret_cast<Dword *>(vertex)[3] = *colour;
    auto number = [](float value) { return static_cast<long double>(value); };
    vertex[0] = float(number(position[0]) / number(position[3]));
    vertex[1] = float(number(position[1]) / number(position[3]));
    vertex[2] = float(number(position[2]) / number(position[3]));
    vertex[4] = float(number(uv[0]) / number(uv[2]));
    vertex[5] = float(number(uv[1]) / number(uv[2]));
    const Dword count = object[80 / 4], primitive = object[0];
    if (static_cast<int>(count + 3) < 16385)
        return;
    if (primitive == 4 && static_cast<int>(count) % 3 == 0) {
        flush_textured(object);
        return;
    }
    if (static_cast<int>(count + 2) < 16385)
        return;
    if (primitive == 2 && static_cast<int>(count) % 2 == 0) {
        flush_textured(object);
        return;
    }
    if (static_cast<int>(count + 1) < 16385)
        return;
    vertices = reinterpret_cast<Dword *>(object[76 / 4]);
    Dword last_colour = vertices[count * 6 - 3];
    // Overflow handling initializes XYZ only; the fourth position component remains uninitialized.
     
    float last_position[4], last_uv[3];
    std::memcpy(last_position, vertices + count * 6 - 6, 12);
    std::memcpy(last_uv, vertices + count * 6 - 2, 8);
    last_uv[2] = 1;
    if (primitive == 5 || primitive == 6) {
        Dword *retained = primitive == 5 ? vertices + (count - 2) * 6 : vertices;
        Dword first_colour = retained[3];
        float first_position[4], first_uv[3];
        std::memcpy(first_position, retained, 12);
        std::memcpy(first_uv, retained + 4, 8);
        first_uv[2] = 1;
        flush_textured(object);
        textured_vertex(object, first_position, &first_colour, first_uv);
        textured_vertex(object, last_position, &last_colour, last_uv);
    } else if (primitive == 3) {
        flush_textured(object);
        textured_vertex(object, last_position, &last_colour, last_uv);
    } else if (primitive == 1 || primitive == ~Dword(0))
        flush_textured(object);
}
} // namespace nfs::graphics::immediate
