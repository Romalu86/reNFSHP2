#include "shape_body_runtime.hpp"
#include "../motion/world_motion_x87.hpp"
namespace nfs::game::runtime::physics {
// Eight corners, face centres, edge centres and four vertical midpoints.
void build_body_vertices(int shape, const float *extent, float *vertices) {
    auto X = [](float value) { return static_cast<long double>(value); };
    const float x = extent[0], y = extent[1], z = extent[2], ny = -y;
    vertices[0] = -x;
    vertices[1] = ny;
    vertices[2] = z;
    vertices[3] = x;
    vertices[4] = ny;
    vertices[5] = z;
    vertices[6] = x;
    vertices[7] = ny;
    vertices[8] = -z;
    vertices[9] = -x;
    vertices[10] = ny;
    vertices[11] = -z;
    if (shape >= 0 && shape <= 5)
        vertices[13] = vertices[16] = vertices[19] = vertices[22] = y;
    switch (shape) {
    case 0:
    case 3:
    case 4:
        vertices[12] = -x;
        vertices[14] = z;
        vertices[15] = x;
        vertices[17] = z;
        vertices[18] = x;
        vertices[20] = -z;
        vertices[21] = -x;
        vertices[23] = -z;
        break;
    case 1:
        vertices[0] = vertices[2] = vertices[3] = vertices[5] = vertices[6] = vertices[8] =
            vertices[9] = vertices[11] = 0;
        vertices[12] = -x;
        vertices[14] = z;
        vertices[15] = x;
        vertices[17] = z;
        vertices[18] = x;
        vertices[20] = -z;
        vertices[21] = -x;
        vertices[23] = -z;
        break;
    case 2:
        if (!(x > z)) {
            vertices[12] = vertices[15] = vertices[21] = vertices[18] = 0;
            vertices[14] = vertices[17] = z;
            vertices[23] = vertices[20] = -z;
        } else {
            vertices[12] = vertices[21] = -x;
            vertices[15] = vertices[18] = x;
            vertices[14] = vertices[17] = vertices[23] = vertices[20] = 0;
        }
        break;
    case 5:
        vertices[12] = vertices[14] = vertices[15] = vertices[17] = vertices[18] = vertices[20] =
            vertices[21] = vertices[23] = 0;
        break;
    default:
        report_body_shape_error(690);
        break;
    }
    vertices[24] = float((X(vertices[12]) + vertices[15] + vertices[3] + vertices[0]) * 0.25f);
    vertices[25] = float((X(ny) + ny + (X(vertices[16]) + vertices[13])) * 0.25f);
    vertices[26] = float((X(vertices[14]) + vertices[17] + vertices[2] + vertices[5]) * 0.25f);
    const float front_z = float(X(vertices[14]) + vertices[17]);
    const float back_x = float(X(vertices[21]) + vertices[18]);
    const float top_y = float(X(vertices[19]) + vertices[22]);
    const auto retained_back_z = X(vertices[23]) + vertices[20];
    const float back_z = float(retained_back_z);
    vertices[27] = float((X(vertices[6]) + vertices[9] + back_x) * 0.25f);
    vertices[28] = float((X(ny) + ny + top_y) * 0.25f);
    vertices[29] = float((retained_back_z + vertices[11] + vertices[8]) * 0.25f);
    vertices[30] = float((X(vertices[21]) + vertices[12] + vertices[9] + vertices[0]) * 0.25f);
    const float left_y = float(X(vertices[22]) + vertices[13]);
    const float left_z = float(X(vertices[23]) + vertices[14]);
    vertices[31] = float((X(ny) + ny + left_y) * 0.25f);
    vertices[32] = float((X(vertices[2]) + vertices[11] + left_z) * 0.25f);
    vertices[33] = float((X(vertices[18]) + vertices[15] + vertices[6] + vertices[3]) * 0.25f);
    const float right_y = float(X(vertices[19]) + vertices[16]);
    vertices[34] = float((X(ny) + ny + right_y) * 0.25f);
    vertices[35] = float((X(vertices[20]) + vertices[17] + vertices[5] + vertices[8]) * 0.25f);
    vertices[36] = float((X(back_x) + vertices[12] + vertices[15]) * 0.25f);
    vertices[37] = float((X(top_y) + vertices[16] + vertices[13]) * 0.25f);
    vertices[38] = float((X(back_z) + vertices[14] + vertices[17]) * 0.25f);
    vertices[39] = float((X(vertices[6]) + vertices[9] + vertices[3] + vertices[0]) * 0.25f);
    const float bottom_y = float(X(ny) + ny);
    vertices[40] = float((X(ny) + ny + bottom_y) * 0.25f);
    vertices[41] = float((X(vertices[11]) + vertices[8] + vertices[2] + vertices[5]) * 0.25f);
    vertices[42] = float((X(vertices[12]) + vertices[15]) * 0.5f);
    vertices[43] = float((X(vertices[16]) + vertices[13]) * 0.5f);
    vertices[44] = float(X(front_z) * 0.5f);
    vertices[45] = float(X(float(X(vertices[18]) + vertices[15])) * 0.5f);
    vertices[46] = float(X(float(X(vertices[19]) + vertices[16])) * 0.5f);
    vertices[47] = float(X(float(X(vertices[20]) + vertices[17])) * 0.5f);
    vertices[48] = float(X(float(X(vertices[21]) + vertices[12])) * 0.5f);
    vertices[49] = float(X(float(X(vertices[22]) + vertices[13])) * 0.5f);
    vertices[50] = float(X(float(X(vertices[23]) + vertices[14])) * 0.5f);
    vertices[51] = float(X(back_x) * 0.5f);
    vertices[52] = float(X(top_y) * 0.5f);
    vertices[53] = float(X(back_z) * 0.5f);
    vertices[54] = float((X(vertices[3]) + vertices[0]) * 0.5f);
    vertices[55] = float(X(bottom_y) * 0.5f);
    vertices[56] = float((X(vertices[2]) + vertices[5]) * 0.5f);
    vertices[57] = float((X(vertices[6]) + vertices[3]) * 0.5f);
    vertices[58] = float(X(bottom_y) * 0.5f);
    vertices[59] = float((X(vertices[5]) + vertices[8]) * 0.5f);
    vertices[60] = float((X(vertices[9]) + vertices[0]) * 0.5f);
    vertices[61] = float(X(bottom_y) * 0.5f);
    vertices[62] = float((X(vertices[2]) + vertices[11]) * 0.5f);
    vertices[63] = float(X(float(X(vertices[6]) + vertices[9])) * 0.5f);
    vertices[64] = float(X(bottom_y) * 0.5f);
    vertices[65] = float(X(float(X(vertices[11]) + vertices[8])) * 0.5f);
    float bottom = ny;
    if (shape == 4) {
        for (unsigned i = 12; i < 24; ++i)
            vertices[i] = float(X(vertices[i]) * 0.707f);
        vertices[0] = float(X(vertices[0]) * 0.707f);
        bottom = float(X(ny) * 0.707f);
        for (unsigned i = 1; i < 12; ++i)
            vertices[i] = i % 3 == 1 ? bottom : float(X(vertices[i]) * 0.707f);
        for (unsigned i = 43; i <= 64; i += 3)
            vertices[i] = float(X(vertices[i]) * 0.707f);
    } else if (shape == 3) {
        for (unsigned i = 12; i < 24; ++i)
            if (i % 3 != 1)
                vertices[i] = float(X(vertices[i]) * 0.707f);
        for (unsigned i = 0; i < 12; ++i)
            if (i % 3 != 1)
                vertices[i] = float(X(vertices[i]) * 0.707f);
    }
    for (unsigned corner = 0; corner < 4; ++corner)
        for (unsigned axis = 0; axis < 3; ++axis)
            vertices[66 + corner * 3 + axis] =
                float((X(vertices[12 + corner * 3 + axis]) + vertices[corner * 3 + axis]) * 0.5f);
}
} // namespace nfs::game::runtime::physics
