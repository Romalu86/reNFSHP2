#pragma once
#include <cstddef>
#include <cstdint>

namespace nfs {

// Pointer words remain uint32_t even in the 64-bit data-validation build.
// Argument words have different meanings for different original operation codes.
struct FileOperation32 {
    std::uint32_t queue_link;           // +00; queue implementation remains unported
    std::uint32_t token;                // +04; device bits 0..4, sequence bits 5..28
    std::uint32_t operation;
    std::uint32_t flags;                // +0c
    std::int8_t status;
    std::uint8_t priority;              // +11
    std::uint16_t unknown_12;           // not assigned invented semantics
    std::uint32_t win32_error;          // +14
    std::uint32_t file;                 // +18; original backend handle object pointer
    std::uint32_t callback_argument;    // +1c; passed as third callback argument
    std::uint32_t callback;
    std::uint32_t argument_24;          // +24; mode, offset or result by operation
    std::uint32_t argument_28;          // +28; byte count or result by operation
    std::uint32_t path_or_buffer;       // +2c
};
static_assert(sizeof(FileOperation32)==48);
static_assert(offsetof(FileOperation32,token)==4);
static_assert(offsetof(FileOperation32,status)==16);
static_assert(offsetof(FileOperation32,priority)==17);
static_assert(offsetof(FileOperation32,callback_argument)==28);
static_assert(offsetof(FileOperation32,callback)==32);
static_assert(offsetof(FileOperation32,path_or_buffer)==44);

std::uint32_t file_operation_key(const FileOperation32& request) noexcept;
}
