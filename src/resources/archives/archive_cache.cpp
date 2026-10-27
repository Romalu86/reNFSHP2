#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/resources/archives/big_directory.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
namespace nfs::files::archives {
Entry entries[9]{};int count=0,next=0;CRITICAL_SECTION lock;
// Same body as , with byte count first.
void* allocate_entry(int bytes,const char* name,Dword flags){void* memory=kernel::memory_allocate(name,bytes,flags);
    if(gui::allocation_break_address&&memory==gui::allocation_break_address)kernel::debug_break();return memory;}
// Failed opens still consume a count slot; eviction does
// not advance next until a new header is successfully cached in the original.
Entry* acquire(const char* path,Dword flags){
    for(int i=0;i<count;++i)if(entries[i].directory&&!std::strcmp(entries[i].name,path))return &entries[i];
    Entry* result=&entries[next];
    if(count<9)++count;
    else if(result->directory){gui::application_release(result->directory);result->directory=nullptr;result->name[0]=0;close_sync(result->file,100);}
    Dword handle;
    if(!open_sync(path,1,100,&handle))return nullptr;
    auto* directory=static_cast<std::uint8_t*>(allocate_entry(2704,path,flags|256));
    if(!directory){close_sync(handle,99);return nullptr;}
    read_sync(handle,0,directory,2704,99);
    int bytes=static_cast<int>(big_directory_size(directory));
    if(bytes>2704){auto* grown=static_cast<std::uint8_t*>(allocate_entry(bytes,path,flags|256));
        if(!grown){gui::application_release(directory);close_sync(handle,99);return nullptr;}
        kernel::memory_copy(grown,directory,2704);gui::application_release(directory);directory=grown;
        read_sync(handle,2704,grown+2704,bytes-2704,99);}
    next=(next+1)%9;std::strcpy(result->name,path);result->directory=directory;result->file=handle;return result;
}
// Comparison is CRT stricmp, not BIG's signed-character
// comparator. Default C-locale bytes are covered; old CRT non-C tables remain open.
void* load(const char* archive,const char* name,Dword flags,std::uint8_t keep_path){
    void* result=nullptr;EnterCriticalSection(&lock);Entry* cached=acquire(archive,flags^256);
    if(cached&&cached->directory&&name){const char* target=name;const char* slash;
        if(!keep_path&&(slash=std::strrchr(name,'\\'))!=nullptr)target=slash+1;else if(*target=='\\')++target;
        int total=static_cast<int>(big_entry_count(cached->directory));
        for(int i=0;i<total;++i){Dword position,bytes;const char* candidate=big_find_entry(cached->directory,nullptr,i,&position,&bytes);
            if(candidate&&!_stricmp(candidate,target)){
                if(position&&bytes&&(result=allocate_entry(bytes,target,flags))!=nullptr){
                    if(read_sync(cached->file,position,result,bytes,100))break;gui::application_release(result);}
                result=nullptr;break;}}
    }
    LeaveCriticalSection(&lock);return result;
}
// No path stripping in this entrypoint.
Dword offset(const char* archive,const char* name,Dword flags){Dword position=~Dword(0),bytes=0;EnterCriticalSection(&lock);
    Entry* cached=acquire(archive,flags^256);
    if(cached){int total=static_cast<int>(big_entry_count(cached->directory));int i=0;
        for(;i<total;++i){const char* candidate=big_find_entry(cached->directory,nullptr,i,&position,&bytes);if(candidate&&!_stricmp(candidate,name))break;}
        if(i>=total)position=~Dword(0);}
    LeaveCriticalSection(&lock);return position;
}
// Continue after failures and retain successful outputs.
Dword load_many(const char* archive,const char*const* names,void** output,int total,Dword flags){Dword result=1;
    for(int i=0;i<total;++i){output[i]=load(archive,names[i],flags,0);if(!output[i])result=0;}return result;}

void initialize(){InitializeCriticalSection(&lock);}

void clear(){for(int i=0;i<count;++i)if(entries[i].directory){gui::application_release(entries[i].directory);entries[i].directory=nullptr;entries[i].name[0]=0;close_sync(entries[i].file,100);}count=0;next=0;}

void shutdown(){clear();DeleteCriticalSection(&lock);}
}
