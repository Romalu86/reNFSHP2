#include "nfs/graphics/transforms/transform_handlers.hpp"
static_assert(__LDBL_MANT_DIG__ == 64, "This translation unit requires native x87 intermediates");
namespace nfs::game::runtime::matrices::handlers {
using namespace matrices::pending;
namespace {
// Shared scalar formulas are kept with each entry's saved addressing/store order.
void project_points(int count, const void *input, void *output, bool indexed, bool rotate,
                    bool shift, bool capture) {
    auto X = [](float value) { return static_cast<long double>(value); };
    do {
        if (indexed) {
            Dword index = *static_cast<Dword *>(saved_vertices);
            saved_vertices = static_cast<Dword *>(saved_vertices) + 1;
            saved_input = static_cast<const Byte *>(input) + index * input_stride;
            saved_output = static_cast<Byte *>(output) + index * output_stride;
            if (capture)
                saved_end = static_cast<Byte *>(vertex_end) + index * 12;
        }
        auto in = [&]() { return static_cast<const volatile float *>(saved_input); };
        auto out = [&]() { return static_cast<volatile float *>(saved_output); };
        float z;
        Dword depth_bits;
        if (rotate) {
            long double sum;
            if (indexed) {
                const auto second = X(matrix[5]) * in()[1];
                const auto first = X(matrix[8]) * in()[2];
                sum = second + first;
            } else {
                const auto first = X(matrix[8]) * in()[2];
                const auto second = X(matrix[5]) * in()[1];
                sum = first + second;
            }
            sum += X(matrix[2]) * in()[0];
            if (shift)
                sum += translation[2];
            z = float(sum);
            depth_bits = at<Dword>(&z, 0);
        } else if (shift) {
            z = float(X(translation[2]) + in()[2]);
            depth_bits = at<Dword>(&z, 0);
        } else {
            // The untransformed path tests the source representation before any FP load.
            depth_bits = at<const volatile Dword>(saved_input, 8);
        }
        const auto reciprocal = depth_bits
                                    ? X(1.0f) / (rotate || shift ? X(z) : X(in()[2]))
                                    : (rotate || shift ? X(10000.0f) : X(100000.0f));
        long double x;
        if (capture && !rotate && !shift) {
            // This one capture component is a raw DWORD copy, followed by an FP reload.
            at<volatile Dword>(saved_end, 0) = at<const volatile Dword>(saved_input, 0);
            x = X(*static_cast<const volatile float *>(saved_end));
        } else {
            if (rotate) {
                if (indexed) {
                    const auto second = X(matrix[3]) * in()[1];
                    const auto first = X(matrix[6]) * in()[2];
                    x = second + first;
                } else {
                    const auto first = X(matrix[6]) * in()[2];
                    const auto second = X(matrix[3]) * in()[1];
                    x = first + second;
                }
                x += X(matrix[0]) * in()[0];
                if (shift)
                    x += translation[0];
            } else
                x = shift ? X(translation[0]) + in()[0] : X(in()[0]);
            if (capture) {
                *static_cast<volatile float *>(saved_end) = float(x);
                x = X(*static_cast<const volatile float *>(saved_end));
            }
        }
        out()[0] = float((!rotate && !shift && !capture ? X(screen_x) * reciprocal * x
                                                        : x * screen_x * reciprocal) +
                         offset_x);
        if (capture)
            saved_end = static_cast<float *>(saved_end) + 1;
        auto y = rotate ? (X(matrix[7]) * in()[2] + X(matrix[4]) * in()[1]) + X(matrix[1]) * in()[0]
                        : (shift ? X(translation[1]) + in()[1] : X(in()[1]));
        if (rotate && shift)
            y += translation[1];
        if (capture) {
            *static_cast<volatile float *>(saved_end) = float(y);
            y = X(*static_cast<const volatile float *>(saved_end));
        }
        out()[1] = float(X(offset_y) - y * screen_y * reciprocal);
        long double depth;
        if (capture) {
            saved_end = static_cast<float *>(saved_end) + 1;
            if (rotate || shift) {
                depth = X(z);
                at<volatile Dword>(saved_end, 0) = depth_bits;
            } else {
                // X/Y stores may have changed the input depth through an overlapping buffer.
                *static_cast<volatile float *>(saved_end) = in()[2];
                depth = X(*static_cast<const volatile float *>(saved_end));
            }
        } else
            depth = rotate || shift ? X(z) : X(in()[2]);
        out()[2] = float((depth - near_plane) * depth_scale);
        if (capture)
            saved_end = static_cast<float *>(saved_end) + 1;
        out()[3] = float(reciprocal);
        if (!indexed) {
            saved_input = static_cast<const Byte *>(saved_input) + input_stride;
            saved_output = static_cast<Byte *>(saved_output) + output_stride;
        }
    } while (--count);
}
} // namespace

void translate(int count, const void *, void *) {
    do {
        for (unsigned i = 0; i < 3; ++i)
            at<float>(saved_output, i * 4) = float(
                static_cast<long double>(translation[i]) + at<float>(saved_input, i * 4));
        saved_input = static_cast<const Byte *>(saved_input) + input_stride;
        saved_output = static_cast<Byte *>(saved_output) + output_stride;
    } while (--count);
}

void translate_indexed(int count, const void *input, void *output) {
    do {
        Dword index = *static_cast<Dword *>(saved_vertices);
        saved_vertices = static_cast<Dword *>(saved_vertices) + 1;
        saved_input = static_cast<const Byte *>(input) + index * input_stride;
        saved_output = static_cast<Byte *>(output) + index * output_stride;
        for (unsigned i = 0; i < 3; ++i)
            at<float>(saved_output, i * 4) = float(
                static_cast<long double>(translation[i]) + at<float>(saved_input, i * 4));
    } while (--count);
}

void project(int n, const void *i, void *o) { project_points(n, i, o, false, false, false, false); }

void project_indexed(int n, const void *i, void *o) {
    project_points(n, i, o, true, false, false, false);
}

void translate_project(int n, const void *i, void *o) {
    project_points(n, i, o, false, false, true, false);
}

void translate_project_indexed(int n, const void *i, void *o) {
    project_points(n, i, o, true, false, true, false);
}

void capture_project(int n, const void *i, void *o) {
    project_points(n, i, o, false, false, false, true);
}

void capture_translate_project(int n, const void *i, void *o) {
    project_points(n, i, o, false, false, true, true);
}

void capture_rotate_project(int n, const void *i, void *o) {
    project_points(n, i, o, false, true, false, true);
}

void capture_affine_project(int n, const void *i, void *o) {
    project_points(n, i, o, false, true, true, true);
}

void capture_project_indexed(int n, const void *i, void *o) {
    project_points(n, i, o, true, false, false, true);
}

void capture_translate_project_indexed(int n, const void *i, void *o) {
    project_points(n, i, o, true, false, true, true);
}

void capture_rotate_project_indexed(int n, const void *i, void *o) {
    project_points(n, i, o, true, true, false, true);
}

void capture_affine_project_indexed(int n, const void *i, void *o) {
    project_points(n, i, o, true, true, true, true);
}
} // namespace nfs::game::runtime::matrices::handlers
