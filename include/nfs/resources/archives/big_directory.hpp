#pragma once
#include <cstdint>

namespace nfs {
// Read-only BIG directory operations. Requires valid, sufficiently long archive and C strings.
// This layer deliberately adds no exceptions, sorting, name normalization or caches.
 
enum class BigKind : std::uint32_t { unknown, compact, bigf, variable };
BigKind big_kind(const std::uint8_t* archive) noexcept;
std::uint32_t big_entry_count(const std::uint8_t* archive) noexcept;
std::uint32_t big_directory_size(const std::uint8_t* archive) noexcept;
std::uint32_t big_trailer(const std::uint8_t* archive, int* version,
                          std::uint32_t* value) noexcept;
std::uint32_t big_read_field(const std::uint8_t* data, int width) noexcept;

 
int big_name_compare(const char* left, const char* right) noexcept;
const char* big_find_entry(const std::uint8_t* archive, const char* name,
                          int index, std::uint32_t* offset,
                          std::uint32_t* size) noexcept;
}
