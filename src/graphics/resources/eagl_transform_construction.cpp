#include "nfs/graphics/math/eagl_math.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::graphics::math {
// Reads and writes remain sequential for overlapping objects.
void from_compact_transform(float *matrix, const float *rotation, const float *position) {
    matrix[0] = rotation[0];
    matrix[1] = rotation[1];
    matrix[2] = rotation[2];
    matrix[3] = 0;
    matrix[4] = rotation[3];
    matrix[5] = rotation[4];
    matrix[6] = rotation[5];
    matrix[7] = 0;
    matrix[8] = rotation[6];
    matrix[9] = rotation[7];
    matrix[10] = rotation[8];
    matrix[11] = 0;
    matrix[12] = position[0];
    matrix[13] = position[1];
    matrix[14] = position[2];
    matrix[15] = 1;
}

void append_translation(float *matrix, float x, float y, float z) {
    Matrix local;
    translation(local, x, y, z);
    append(matrix, local);
}
} // namespace nfs::graphics::math
