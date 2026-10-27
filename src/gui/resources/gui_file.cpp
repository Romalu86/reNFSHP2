#include "nfs/gui/resources/gui_file.hpp"
#include "nfs/resources/files/filesystem.hpp"
#include <cstring>
namespace nfs::gui {
// deleting destructor .
static void* __fastcall file_delete(File* f,void*,unsigned flags){file_destruct(f);if(flags&1)release(f);return f;}
static const Destroy file_methods[]={reinterpret_cast<Destroy>(file_delete)};

void file_destruct(File* f){f->methods=file_methods;file_close(f);f->methods=base_methods;}

void file_construct(File* f,const char* name,std::uint8_t flags,int priority){Dword mode=0;f->methods=file_methods;f->file=0;f->priority=priority;
    if(flags&4)mode=4;if(flags&2)mode|=2;if(!(flags&7))mode=1;files::open_sync(name,mode,priority,&f->file);f->offset=0;}

void file_close(File* f){if(f->file){files::close_sync(f->file,f->priority);f->file=0;f->offset=0;}}

File *file_write(File *file, const String *text) {
    const char *data = string_ascii(text);
    if (file->file && text->length && data) {
        const int previous = file->offset;
        file->offset = int(files::write_sync(file->file, previous, data, text->length,
                                            file->priority) + Dword(previous));
    }
    return file;
}

File *file_write(File *file, const char *text) {
    if (text && file->file) {
        const Dword length = Dword(std::strlen(text));
        if (length) {
            const int previous = file->offset;
            file->offset = int(files::write_sync(file->file, previous, text, length,
                                                file->priority) + Dword(previous));
        }
    }
    return file;
}
}
