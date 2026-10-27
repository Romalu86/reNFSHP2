#include "nfs/core/application/application.hpp"
#include "nfs/core/console/console.hpp"
#include "nfs/resources/files/filesystem.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>
namespace nfs::kernel {
 
const char *allocation_name(const void *) {
    return nullptr;
}
// Flag0800 stores the annotation after the allocation.
const char *allocation_annotation(const void *memory) {
    const auto *block = static_cast<const Block *>(memory) - 1;
    if (!(block->flags & 2048))
        return nullptr;
    return *reinterpret_cast<const char *const *>(static_cast<const std::uint8_t *>(memory) +
                                                  block->size + 4);
}
using MemoryReport = void(__cdecl *)(const char *, ...);
// Traversal, validation and callback order follow instructions.
void report_pool(Dword pool, const char *format, MemoryReport output) {
    static const char *const validation[] = {
        "BAD", "ok", "-",
        "--------------------------------------------------------------------------------------\n"};
    static const char separator[] =
        "--------------------------------------------------------------------------------------\n";
    output(
        "Name                                Address        Size      Actual Type  CRC Sentinel\n");
    output(separator);
    Pool *owner = pools[pool & 63];
    if (!owner) {
        output("Not initialized\n");
        return;
    }
    Dword free_bytes = 0;
    std::uint16_t crc = 0;
    for (Block *block = owner->first; block; block = block->next) {
        auto *memory = reinterpret_cast<std::uint8_t *>(block) + 16;
        const char *name = allocation_name(memory);
        if (!name)
            name = "";
        const char *annotation = allocation_annotation(memory);
        if (!annotation)
            annotation = "";
        if (block->flags & 16384) {
            if (block->size > 16 && !(block->flags & 32768))
                crc = std::uint16_t(checksum16(memory + 8, block->size - 24, 64490));
            free_bytes += Dword(block->size);
            const unsigned validity = memory_validate_block(block);
            output(format, "   SPACE ", memory, block->size,
                   reinterpret_cast<Dword>(block->next) - reinterpret_cast<Dword>(block),
                   unsigned(block->flags), unsigned(crc), validation[validity], "");
        } else {
            crc = 0;
            if (block->size && !(block->flags & 32768))
                crc = std::uint16_t(checksum16(memory, block->size, 64490));
            const unsigned validity = memory_validate_block(block);
            output(format, name, memory, block->size,
                   reinterpret_cast<Dword>(block->next) - reinterpret_cast<Dword>(block),
                   unsigned(block->flags), unsigned(crc), validation[validity], annotation);
        }
        output("\n");
    }
    output("\nTOTAL FREE MEMORY: %ld ($%08lx)\n", free_bytes, free_bytes);
    output(separator);
}
} // namespace nfs::kernel
namespace nfs::application {
namespace {
alignas(4) Byte report_storage[261]{};
char (&report_filename)[256] = *reinterpret_cast<char (*)[256]>(report_storage);
Dword &report_position = *reinterpret_cast<Dword *>(report_storage + 256);
Byte &report_open = report_storage[260];
Dword report_handle = Dword(-1);

void __cdecl write_pool_line(const char *format, ...) {
    char line[512];
    va_list arguments;
    va_start(arguments, format);
    std::vsprintf(line, format, arguments);
    va_end(arguments);
    kernel::print::write(2, line);
}
// PRINT supplies a writable formatting buffer.
void __cdecl write_report_file(int, const char *message) {
    const Dword bytes = Dword(std::strlen(message));
    bool append_lf = false;
    if (bytes > 1 && (message[bytes - 2] != '\r' || message[bytes - 1] != '\n') &&
        message[bytes - 1] == '\n') {
        const_cast<char *>(message)[bytes - 1] = '\r';
        append_lf = true;
    }
    if (!report_open) {
        if (!files::open_sync(report_filename, 6, 100, &report_handle)) {
            report_open = 0;
            kernel::print::check_console_write("Could not open %s file.\n", report_filename);
            return;
        }
        report_open = 1;
        report_position = 0;
    }
    report_position += files::write_sync(report_handle, report_position, message, bytes, 100);
    if (append_lf)
        report_position += files::write_sync(report_handle, report_position, "\n", 1, 100);
}
} // namespace
namespace pending {
// This report is part of the shipped shutdown path.
void write_memory_report(const char *name) {
    report_open = 0;
    std::sprintf(report_filename, ".\\%s.TXT", name);
    kernel::print::register_device(4, name, write_report_file);
    kernel::print::set_device(4, 1);
    kernel::print::write(2, "MEMCOUNT\r\n");
    constexpr const char *format = "%s,%lx,%lu,%lu,%x,%x,%s,%s\r";
    kernel::report_pool(0, format, write_pool_line);
    for (Dword pool : {lifecycle::track_pool, lifecycle::level_pool, lifecycle::cars_pool,
                      gui::application_pool, lifecycle::audio_pool, lifecycle::character_pool,
                      lifecycle::replay_pool, lifecycle::debug_pool, lifecycle::ui_pool,
                      ini::load_pool, gui::pool, lifecycle::screenshot_pool})
        if (pool)
            kernel::report_pool(pool, format, write_pool_line);
    if (report_open) {
        if (!files::close_sync(report_handle, 100))
            kernel::print::check_console_write("Could not close %s file.\n", report_filename);
        kernel::memory_zero(report_filename, 256);
    }
    report_open = 0;
}
} // namespace pending
} // namespace nfs::application
namespace nfs::lifecycle::phase_dependencies {
// Release console output is RET; pool queries still execute.
void report_memory_usage() {
    kernel::memory_largest_free(0);
    for (Dword pool : {audio_pool, track_pool, level_pool, cars_pool, gui::application_pool,
                      character_pool, replay_pool, debug_pool, ini::load_pool, ui_pool, save_pool})
        if (static_cast<int>(pool) >= 0)
            kernel::memory_largest_free(pool);
}
} // namespace nfs::lifecycle::phase_dependencies
