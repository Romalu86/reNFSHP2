#include "nfs/resources/files/filesystem.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/archives/big_directory.hpp"
#include <cstring>
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::files::backend {
Handle* handles=nullptr;int handle_count=0;
static kernel::Lock table_lock{};
// Operations retain the index, then reload the table at each boundary.
static volatile Handle* current_handle(Dword index) {
    Handle* volatile& table = handles;
    return reinterpret_cast<Handle*>(reinterpret_cast<Dword>(table) + index * sizeof(Handle));
}
static Handle* get(Dword token){if(handles&&static_cast<int>(token)<0&&static_cast<int>(~token)<handle_count){auto* h=handles+~token;if(h->used)return h;}SetLastError(ERROR_INVALID_HANDLE);return nullptr;}
void initialize(int count,void* storage){if(handles)shutdown();handle_count=count;handles=static_cast<Handle*>(storage);
    kernel::memory_zero(storage,Dword(count)*32);kernel::lock_init(&table_lock);}
void shutdown(){if(handles){for(int i=0;i<handle_count;++i)if(handles[i].used)close(~Dword(i));handles=nullptr;handle_count=0;kernel::lock_destroy(&table_lock);}}
int reserve(){kernel::lock_enter(&table_lock);int i=0;
    while(i<handle_count&&handles[i].used)++i;
    if(i<handle_count){kernel::memory_zero(handles+i,32);current_handle(Dword(i))->used=1;current_handle(Dword(i))->file=INVALID_HANDLE_VALUE;}
    kernel::lock_leave(&table_lock);return i;}
int path_drive(const char* path){

    if(!std::strchr(path,'|')){char full[261];char* part=nullptr;DWORD length=GetFullPathNameA(path?path:".",261,full,&part);
        CharUpperA(full);if(static_cast<int>(length)>1&&full[1]==':')return static_cast<signed char>(full[0]);}return 0;
}
std::uint8_t hint(const char* path){if(static_cast<std::uint8_t>(cache::enabled)&&path[0]!='\\'&&path[0]!='/'&&path[1]!=':'&&cache::database){
    if(path[0]=='.')path+=2;auto* entry=static_cast<cache::Entry*>(kernel::binary_search(path,cache::database+1,cache::database->count,261,cache::compare_entry));
    if(entry)return entry->cached;}return 0;
}
Dword open(const char* path, Dword flags, Dword* output) {
    const Dword index = Dword(reserve());
    *output = 0;
    SetLastError(0);
    const Dword read_only = flags & 1;
    const DWORD access = read_only ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE;
    const DWORD share = read_only ? 1 : 0;
    const DWORD creation = (flags & 2) ? ((flags & 4) ? 2 : 1) : ((flags & 4) ? 5 : 3);
    DWORD attributes = 128;
    if (flags & 16)
        attributes = 536871040;
    if (flags & 32)
        attributes |= 134217728;
    current_handle(index)->flags = flags;
    const int drive_id = path_drive(path);
    current_handle(index)->device = static_cast<std::int8_t>(drive_id ? drive_id - 64 : 0);
    if (!(flags & 64) && hint(path)) {
        current_handle(index)->device = static_cast<std::int8_t>(cache::settings.destination[0] - 64);
        char redirected[260];
        std::strcpy(redirected, cache::settings.destination);
        std::strcat(redirected, path);
        HANDLE file = CreateFileA(redirected, access, share, nullptr, creation, attributes, nullptr);
        current_handle(index)->file = file;
    }
    if (current_handle(index)->file == INVALID_HANDLE_VALUE) {
        HANDLE file = CreateFileA(path, access, share, nullptr, creation, attributes, nullptr);
        current_handle(index)->file = file;
    }
    char drive[] = " :\\";
    drive[0] = static_cast<char>(current_handle(index)->device + '@');
    DWORD sectors, sector_bytes, free_clusters, clusters;
    if (!GetDiskFreeSpaceA(drive, &sectors, &sector_bytes, &free_clusters, &clusters))
        sector_bytes = 2048;
    volatile Handle* handle = current_handle(index);
    if (handle->file == INVALID_HANDLE_VALUE) {
        handle->used = 0;
        return 0;
    }
    handle->chunk = sector_bytes ? sector_bytes : 16384;
    DWORD length = GetFileSize(current_handle(index)->file, nullptr);
    current_handle(index)->length = static_cast<int>(length);
    Dword result = 1;
    if (!(flags & 8)) {
        handle = current_handle(index);
        handle->flags &= ~8u;
        current_handle(index)->mapping = INVALID_HANDLE_VALUE;
        current_handle(index)->data = nullptr;
    } else {
        HANDLE mapping = CreateFileMappingA(current_handle(index)->file, nullptr,
                                            read_only ? 134217730 : 134217732, 0, 0, nullptr);
        current_handle(index)->mapping = mapping;
        handle = current_handle(index);
        mapping = handle->mapping;
        if (mapping) {
            void* mapped = MapViewOfFile(mapping, read_only ? 4 : 2, 0, 0, handle->length);
            current_handle(index)->data = static_cast<std::uint8_t*>(mapped);
            handle = current_handle(index);
        }
        if (!handle->data) {
            result = 0;
            handle->flags &= ~8u;
        }
    }
    *output = ~index;
    return result;
}
Dword information(Dword token, Dword* flags, Dword* size) {
    volatile Handle* handle = get(token);
    if (!handle)
        return 0;
    if (flags) {
        *flags = handle->flags;
        handle = current_handle(~token);
    }
    if (size)
        *size = Dword(handle->length);
    return 1;
}
int read(Dword token, void* data, int bytes) {
    if (!get(token))
        return 0;
    const Dword index = ~token;
    SetLastError(0);
    volatile Handle* handle = current_handle(index);
    Dword total = 0;
    const int length = handle->length;
    if (length < handle->position) {
        handle->position = length;
        handle = current_handle(index);
    }
    const int position = handle->position;
    const int available = handle->length;
    Dword remaining = Dword(bytes);
    if (available < int(Dword(position) + remaining))
        remaining = Dword(available) - Dword(position);
    Dword chunk = handle->chunk;
    handle->reading = 1;
    handle = current_handle(index);
    auto* destination = static_cast<std::uint8_t*>(data);
    while (remaining && handle->reading) {
        if (remaining < chunk)
            chunk = remaining;
        DWORD actual;
        std::uint8_t* memory = handle->data;
        if (memory) {
            auto* source = reinterpret_cast<const void*>(reinterpret_cast<Dword>(memory) +
                                                        Dword(handle->position));
            kernel::memory_copy(destination, source, chunk);
            actual = chunk;
            kernel::thread_yield(0);
        } else if (!ReadFile(handle->file, destination, chunk, &actual, nullptr)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                handle = current_handle(index);
                total = 0;
                break;
            }
            kernel::thread_yield(1);
            handle = current_handle(index);
            continue;
        }
        handle = current_handle(index);
        handle->position = int(Dword(handle->position) + actual);
        handle = current_handle(index);
        total += actual;
        destination = reinterpret_cast<std::uint8_t*>(reinterpret_cast<Dword>(destination) + actual);
        const int end = handle->length;
        if (end < handle->position) {
            handle->position = end;
            handle = current_handle(index);
        }
        if (actual < chunk)
            break;
        remaining -= actual;
    }
    handle->reading = 0;
    return int(total);
}
int write(Dword token, const void* data, int bytes) {
    if (!get(token))
        return 0;
    const Dword index = ~token;
    SetLastError(0);
    volatile Handle* handle = current_handle(index);
    Dword total = 0;
    const int length = handle->length;
    if (length < handle->position) {
        handle->position = length;
        handle = current_handle(index);
    }
    std::uint8_t* memory = handle->data;
    Dword chunk = handle->chunk;
    Dword remaining = Dword(bytes);
    if (memory) {
        const int position = handle->position;
        const int end = handle->length;
        if (end < int(Dword(position) + remaining))
            remaining = Dword(end) - Dword(position);
    }
    auto* source = static_cast<const std::uint8_t*>(data);
    while (remaining) {
        if (remaining < chunk)
            chunk = remaining;
        DWORD actual;
        memory = handle->data;
        if (memory) {
            auto* destination = reinterpret_cast<void*>(reinterpret_cast<Dword>(memory) +
                                                       Dword(handle->position));
            kernel::memory_copy(destination, source, chunk);
            actual = chunk;
            kernel::thread_yield(0);
        } else if (!WriteFile(handle->file, source, chunk, &actual, nullptr)) {
            if (GetLastError() != ERROR_IO_PENDING)
                return 0;
            kernel::thread_yield(1);
            handle = current_handle(index);
            continue;
        }
        handle = current_handle(index);
        handle->position = int(Dword(handle->position) + actual);
        total += actual;
        source = reinterpret_cast<const std::uint8_t*>(reinterpret_cast<Dword>(source) + actual);
        remaining = actual < chunk ? 0 : remaining - actual;
        handle = current_handle(index);
        const int position = handle->position;
        if (handle->length < position) {
            handle->length = position;
            handle = current_handle(index);
        }
    }
    return int(total);
}
Dword seek(Dword token, int position) {
    if (!get(token))
        return 0;
    const Dword index = ~token;
    SetLastError(0);
    current_handle(index)->position = position;
    volatile Handle* handle = current_handle(index);
    const int cursor = handle->position;
    const int length = handle->length;
    if (length < cursor) {
        handle->position = length;
        handle = current_handle(index);
    } else if (cursor < 0) {
        handle->position = 0;
        handle = current_handle(index);
    }
    if (!handle->data) {
        DWORD actual = SetFilePointer(handle->file, handle->position, nullptr, FILE_BEGIN);
        current_handle(index)->position = static_cast<int>(actual);
    }
    return 1;
}
Dword close(Dword token) {
    if (!get(token))
        return 0;
    const Dword index = ~token;
    SetLastError(0);
    volatile Handle* handle = current_handle(index);
    if (handle->mapping != INVALID_HANDLE_VALUE) {
        UnmapViewOfFile(handle->data);
        CloseHandle(current_handle(index)->mapping);
        handle = current_handle(index);
    }
    HANDLE file = handle->file;
    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
        handle = current_handle(index);
    }
    handle->used = 0;
    return 1;
}
Dword erase(const char* path) {
    SetLastError(0);
    return Dword(DeleteFileA(path));
}
Dword memory_file(void* data, Dword size, std::uint8_t device, Dword* result) {
    const Dword index = Dword(reserve());
    current_handle(index)->device = static_cast<std::int8_t>(device);
    current_handle(index)->file = INVALID_HANDLE_VALUE;
    current_handle(index)->flags = 8;
    current_handle(index)->chunk = 16384;
    current_handle(index)->mapping = INVALID_HANDLE_VALUE;
    current_handle(index)->data = static_cast<std::uint8_t*>(data);
    current_handle(index)->position = 0;
    current_handle(index)->length = int(size);
    *result = ~index;
    return 1;
}
void* data(Dword token){auto* h=get(token);return h?h->data:nullptr;}
Dword truncate(Dword token, int position) {
    if (!get(token))
        return 0;
    const Dword index = ~token;
    SetLastError(0);
    volatile Handle* handle = current_handle(index);
    if (handle->data)
        return 0;
    // The entry truncates at the OS cursor before seeking to the argument.
    const BOOL result = SetEndOfFile(handle->file);
    DWORD actual = SetFilePointer(current_handle(index)->file, position, nullptr, FILE_BEGIN);
    current_handle(index)->position = static_cast<int>(actual);
    return Dword(result);
}
int cancel_read(Dword token){auto* h=get(token);if(!h)return 0;int reading=h->reading;if(reading)h->reading=0;return reading;}
int device(Dword token){auto* h=get(token);return h?h->device:0;}
bool is_memory(Dword token){auto* h=get(token);return h&&h->file==INVALID_HANDLE_VALUE;}
}
