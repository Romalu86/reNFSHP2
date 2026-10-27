#include "d3dx_dispatch_kernels.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::graphics::d3dx {
using kernel::numeric::Extended;

float *__stdcall invert_matrix_scalar(float *out, float *determinant, const float *matrix) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    float m[16];
    for (unsigned i = 0; i < 16; ++i)
        m[i] = matrix[i];
    float top01 = float(X(m[5]) * m[0] - X(m[4]) * m[1]);
    float top02 = float(X(m[9]) * m[0] - X(m[8]) * m[1]);
    float top03 = float(X(m[13]) * m[0] - X(m[12]) * m[1]);
    float top12 = float(X(m[9]) * m[4] - X(m[8]) * m[5]);
    float top13 = float(X(m[13]) * m[4] - X(m[12]) * m[5]);
    float top23 = float(X(m[13]) * m[8] - X(m[12]) * m[9]);
    float cofactor[16];
    cofactor[15] = float(X(m[2]) * top12 - X(m[6]) * top02 + X(m[10]) * top01);
    cofactor[14] = float(X(m[6]) * top03 - X(m[14]) * top01 - X(m[2]) * top13);
    cofactor[13] = float(X(m[2]) * top23 - X(m[10]) * top03 + X(m[14]) * top02);
    cofactor[12] = float(X(m[10]) * top13 - X(m[14]) * top12 - X(m[6]) * top23);
    cofactor[11] = float(X(top02) * m[7] - X(top01) * m[11] - X(top12) * m[3]);
    cofactor[10] = float(X(top13) * m[3] - X(top03) * m[7] + X(top01) * m[15]);
    cofactor[9] = float(X(top03) * m[11] - X(top02) * m[15] - X(top23) * m[3]);
    cofactor[8] = float(X(top23) * m[7] - X(top13) * m[11] + X(top12) * m[15]);
    float bottom01 = float(X(m[2]) * m[7] - X(m[6]) * m[3]);
    float bottom02 = float(X(m[2]) * m[11] - X(m[10]) * m[3]);
    auto bottom03 = X(m[2]) * m[15] - X(m[14]) * m[3];
    auto bottom12 = X(m[6]) * m[11] - X(m[10]) * m[7];
    auto bottom13 = X(m[6]) * m[15] - X(m[14]) * m[7];
    auto bottom23 = X(m[10]) * m[15] - X(m[14]) * m[11];
    cofactor[3] = float(X(bottom02) * m[5] - X(bottom01) * m[9] - bottom12 * m[1]);
    cofactor[2] = float(bottom13 * m[1] - bottom03 * m[5] + X(bottom01) * m[13]);
    cofactor[1] = float(bottom03 * m[9] - X(bottom02) * m[13] - bottom23 * m[1]);
    cofactor[0] = float(bottom23 * m[5] - bottom13 * m[9] + bottom12 * m[13]);
    cofactor[7] = float(bottom12 * m[0] - X(bottom02) * m[4] + X(bottom01) * m[8]);
    cofactor[6] = float(bottom03 * m[4] - X(bottom01) * m[12] - bottom13 * m[0]);
    cofactor[5] = float(X(m[0]) * bottom23 - bottom03 * m[8] + X(bottom02) * m[12]);
    cofactor[4] = float(bottom13 * m[8] - bottom12 * m[12] - bottom23 * m[4]);
    auto value = X(cofactor[3]) * m[12] + X(cofactor[2]) * m[8] + X(cofactor[1]) * m[4] +
                 X(cofactor[0]) * m[0];
    if (determinant)
        *determinant = float(value);
    float inverse = float(X(1) / value);

    if (!std::isfinite(double(inverse)))
        return nullptr;
    for (unsigned i = 0; i < 16; ++i)
        out[i] = float(X(cofactor[i]) * inverse);
    return out;
}
} // namespace nfs::graphics::d3dx
