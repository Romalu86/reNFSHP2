#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/network/packets/network_quantizers.hpp"
#include <cstdlib>
namespace nfs::network::bits {
namespace pending {
alignas(16) Quantizer position_x_quantizer{};
alignas(16) Quantizer position_y_quantizer{};
alignas(16) Quantizer position_z_quantizer{};
} // namespace pending
namespace {
alignas(16) Quantizer orientation_x{};
alignas(16) Quantizer orientation_y{};
alignas(16) Quantizer orientation_z{};
alignas(16) Quantizer orientation_w{};
alignas(16) Quantizer linear_x{};
alignas(16) Quantizer linear_y{};
alignas(16) Quantizer linear_z{};
alignas(16) Quantizer angular_x{};
alignas(16) Quantizer angular_y{};
alignas(16) Quantizer angular_z{};
std::uint8_t first_finalized{}, second_finalized{};
} // namespace CRT destructor guard.
 
void finalize_position_x() {
    if (!(first_finalized & 1)) {
        first_finalized |= 1;
        destroy_quantizer(&pending::position_x_quantizer);
    }
}
// CRT initializer; wired in the array below.
void initialize_position_x() {
    construct_quantizer(&pending::position_x_quantizer);
    std::atexit(finalize_position_x);
}
// CRT destructor guard.
void finalize_position_y() {
    if (!(first_finalized & 2)) {
        first_finalized |= 2;
        destroy_quantizer(&pending::position_y_quantizer);
    }
}
// CRT initializer; wired in the array below.
void initialize_position_y() {
    construct_quantizer(&pending::position_y_quantizer);
    std::atexit(finalize_position_y);
}
// CRT destructor guard.
void finalize_position_z() {
    if (!(first_finalized & 4)) {
        first_finalized |= 4;
        destroy_quantizer(&pending::position_z_quantizer);
    }
}
// CRT initializer; wired in the array below.
void initialize_position_z() {
    construct_quantizer(&pending::position_z_quantizer);
    std::atexit(finalize_position_z);
}
// CRT destructor guard.
void finalize_orientation_x() {
    if (!(first_finalized & 8)) {
        first_finalized |= 8;
        destroy_quantizer(&orientation_x);
    }
}
// CRT initializer; wired in the array below.
void initialize_orientation_x() {
    construct_quantizer(&orientation_x);
    std::atexit(finalize_orientation_x);
}
// CRT destructor guard.
void finalize_orientation_y() {
    if (!(first_finalized & 16)) {
        first_finalized |= 16;
        destroy_quantizer(&orientation_y);
    }
}
// CRT initializer; wired in the array below.
void initialize_orientation_y() {
    construct_quantizer(&orientation_y);
    std::atexit(finalize_orientation_y);
}
// CRT destructor guard.
void finalize_orientation_z() {
    if (!(first_finalized & 32)) {
        first_finalized |= 32;
        destroy_quantizer(&orientation_z);
    }
}
// CRT initializer; wired in the array below.
void initialize_orientation_z() {
    construct_quantizer(&orientation_z);
    std::atexit(finalize_orientation_z);
}
// CRT destructor guard.
void finalize_orientation_w() {
    if (!(first_finalized & 64)) {
        first_finalized |= 64;
        destroy_quantizer(&orientation_w);
    }
}
// CRT initializer; wired in the array below.
void initialize_orientation_w() {
    construct_quantizer(&orientation_w);
    std::atexit(finalize_orientation_w);
}
// CRT destructor guard.
void finalize_linear_x() {
    if (!(first_finalized & 128)) {
        first_finalized |= 128;
        destroy_quantizer(&linear_x);
    }
}
// CRT initializer; wired in the array below.
void initialize_linear_x() {
    construct_quantizer(&linear_x);
    std::atexit(finalize_linear_x);
}
// CRT destructor guard.
void finalize_linear_y() {
    if (!(second_finalized & 1)) {
        second_finalized |= 1;
        destroy_quantizer(&linear_y);
    }
}
// CRT initializer; wired in the array below.
void initialize_linear_y() {
    construct_quantizer(&linear_y);
    std::atexit(finalize_linear_y);
}
// CRT destructor guard.
void finalize_linear_z() {
    if (!(second_finalized & 2)) {
        second_finalized |= 2;
        destroy_quantizer(&linear_z);
    }
}
// CRT initializer; wired in the array below.
void initialize_linear_z() {
    construct_quantizer(&linear_z);
    std::atexit(finalize_linear_z);
}
// CRT destructor guard.
void finalize_angular_x() {
    if (!(second_finalized & 4)) {
        second_finalized |= 4;
        destroy_quantizer(&angular_x);
    }
}
// CRT initializer; wired in the array below.
void initialize_angular_x() {
    construct_quantizer(&angular_x);
    std::atexit(finalize_angular_x);
}
// CRT destructor guard.
void finalize_angular_y() {
    if (!(second_finalized & 8)) {
        second_finalized |= 8;
        destroy_quantizer(&angular_y);
    }
}
// CRT initializer; wired in the array below.
void initialize_angular_y() {
    construct_quantizer(&angular_y);
    std::atexit(finalize_angular_y);
}
// CRT destructor guard.
void finalize_angular_z() {
    if (!(second_finalized & 16)) {
        second_finalized |= 16;
        destroy_quantizer(&angular_z);
    }
}
// CRT initializer; wired in the array below.
void initialize_angular_z() {
    construct_quantizer(&angular_z);
    std::atexit(finalize_angular_z);
}
// All 13 descriptors, including the requested three positions.
void configure_quantizers() {
    configure_bits(&pending::position_x_quantizer, "m_positionX", -8192.0f, 8192.0f, 24);
    configure_bits(&pending::position_y_quantizer, "m_positionY", -8192.0f, 8192.0f, 24);
    configure_bits(&pending::position_z_quantizer, "m_positionZ", -8192.0f, 8192.0f, 24);
    configure_bits(&orientation_x, "m_orientQuatX", -1.0f, 1.0f, 20);
    configure_bits(&orientation_y, "m_orientQuatY", -1.0f, 1.0f, 20);
    configure_bits(&orientation_z, "m_orientQuatZ", -1.0f, 1.0f, 20);
    configure_bits(&orientation_w, "m_orientQuatW", -1.0f, 1.0f, 20);
    configure_bits(&linear_x, "m_linearVelX", -512.0f, 512.0f, 20);
    configure_bits(&linear_y, "m_linearVelY", -256.0f, 256.0f, 16);
    configure_bits(&linear_z, "m_linearVelZ", -512.0f, 512.0f, 20);
    configure_bits(&angular_x, "m_angularVelX", -1.0f, 1.0f, 12);
    configure_bits(&angular_y, "m_angularVelY", -1.0f, 1.0f, 12);
    configure_bits(&angular_z, "m_angularVelZ", -1.0f, 1.0f, 12);
    pending::position_x_quantizer.bytes[332] = 1;
}
} // namespace nfs::network::bits


NFS_KERNEL_INITIALIZER(startup_initialize_position_x, ".CRT$XCU_NFS_006901F4",
                       nfs::network::bits::initialize_position_x);
NFS_KERNEL_INITIALIZER(startup_initialize_position_y, ".CRT$XCU_NFS_006901F8",
                       nfs::network::bits::initialize_position_y);
NFS_KERNEL_INITIALIZER(startup_initialize_position_z, ".CRT$XCU_NFS_006901FC",
                       nfs::network::bits::initialize_position_z);
NFS_KERNEL_INITIALIZER(startup_initialize_orientation_x, ".CRT$XCU_NFS_00690200",
                       nfs::network::bits::initialize_orientation_x);
NFS_KERNEL_INITIALIZER(startup_initialize_orientation_y, ".CRT$XCU_NFS_00690204",
                       nfs::network::bits::initialize_orientation_y);
NFS_KERNEL_INITIALIZER(startup_initialize_orientation_z, ".CRT$XCU_NFS_00690208",
                       nfs::network::bits::initialize_orientation_z);
NFS_KERNEL_INITIALIZER(startup_initialize_orientation_w, ".CRT$XCU_NFS_0069020C",
                       nfs::network::bits::initialize_orientation_w);
NFS_KERNEL_INITIALIZER(startup_initialize_linear_x, ".CRT$XCU_NFS_00690210",
                       nfs::network::bits::initialize_linear_x);
NFS_KERNEL_INITIALIZER(startup_initialize_linear_y, ".CRT$XCU_NFS_00690214",
                       nfs::network::bits::initialize_linear_y);
NFS_KERNEL_INITIALIZER(startup_initialize_linear_z, ".CRT$XCU_NFS_00690218",
                       nfs::network::bits::initialize_linear_z);
NFS_KERNEL_INITIALIZER(startup_initialize_angular_x, ".CRT$XCU_NFS_0069021C",
                       nfs::network::bits::initialize_angular_x);
NFS_KERNEL_INITIALIZER(startup_initialize_angular_y, ".CRT$XCU_NFS_00690220",
                       nfs::network::bits::initialize_angular_y);
NFS_KERNEL_INITIALIZER(startup_initialize_angular_z, ".CRT$XCU_NFS_00690224",
                       nfs::network::bits::initialize_angular_z);
