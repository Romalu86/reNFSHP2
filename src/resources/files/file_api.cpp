#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/archives/big_directory.hpp"
#include <cstring>
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::files::file {
static BuildInfo build{513,3,"Lchan, Friday 04:22PM Sep 20, 2002","EA Internal release"};
BuildInfo* build_info(){return &build;}
int priority=100;
struct Arguments {const char* path;void* buffer;Dword size;int fatal;Dword flags;};
static Dword __cdecl length_atomic(int level,Dword context){
    auto* a=reinterpret_cast<Arguments*>(context);Dword handle;
    if(open_sync(a->path,1,level,&handle)){Dword result=size_sync(handle,level-1);close_sync(handle,level-1);return result;}
    if(a->fatal)kernel::error("FILE_size - unable to open file %s\n",a->path);return 0;
}
Dword length(const char* path){Arguments a;a.path=path;a.fatal=1;return atomic(length_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a));}
Dword try_length(const char* path){Arguments a;a.path=path;a.fatal=0;return atomic(length_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a));}
static Dword __cdecl load_atomic(int level,Dword context){
    auto* a=reinterpret_cast<Arguments*>(context);Dword handle;
    if(!open_sync(a->path,1,level,&handle)){if(a->fatal)kernel::error("FILE_load - unable to open file %s\n",a->path);return 0;}
    --level;Dword bytes=size_sync(handle,level);void* result=allocate(a->path,bytes,a->flags);
    if(result){read_sync(handle,0,result,bytes,level);if((a->flags&1)&&backend::is_memory(handle))a->flags&=4294901759u;
        close_sync(handle,level);if(a->flags&65536)cache::store(a->path,result,bytes,200);return reinterpret_cast<Dword>(result);}
    if(a->fatal)kernel::error("FILE_load - unable to reserve memory for file %s\n",a->path);close_sync(handle,level);return 0;
}
void* load(const char* path,Dword flags){Arguments a;a.path=path;a.fatal=1;a.flags=flags;return reinterpret_cast<void*>(atomic(load_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a)));}
void* try_load(const char* path,Dword flags){Arguments a;a.path=path;a.fatal=0;a.flags=flags;return reinterpret_cast<void*>(atomic(load_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a)));}
static Dword __cdecl load_length_atomic(int level,Dword context){
    auto* a=reinterpret_cast<Arguments*>(context);Dword handle;
    if(!open_sync(a->path,1,level,&handle)){if(a->fatal)kernel::error("FILE_load - unable to open file %s\n",a->path);return 0;}
    --level;Dword bytes=size_sync(handle,level);void* result=allocate(a->path,bytes,a->flags);
    if(result){read_sync(handle,0,result,bytes,level);if((a->flags&1)&&backend::is_memory(handle))a->flags&=4294901759u;
        close_sync(handle,level);if(a->flags&65536)cache::store(a->path,result,bytes,200);a->size=bytes;return reinterpret_cast<Dword>(result);}
    if(a->fatal)kernel::error("FILE_load - unable to reserve memory for file %s\n",a->path);close_sync(handle,level);return 0;
}
void* load_with_length(const char* path,Dword* bytes,Dword flags){Arguments a;a.path=path;a.fatal=1;a.flags=flags;
    Dword result=atomic(load_length_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a));if(bytes)*bytes=a.size;return reinterpret_cast<void*>(result);}
void* try_load_with_length(const char* path,Dword* bytes,Dword flags){Arguments a;a.path=path;a.fatal=0;a.flags=flags;
    Dword result=atomic(load_length_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a));if(bytes)*bytes=a.size;return reinterpret_cast<void*>(result);}
static Dword __cdecl into_atomic(int level,Dword context){
    auto* a=reinterpret_cast<Arguments*>(context);int bytes=static_cast<int>(a->size);if(bytes<=0)bytes=2147483647;Dword handle;
    if(!open_sync(a->path,1,level,&handle))return 0;--level;Dword actual=read_sync(handle,0,a->buffer,bytes,level);

    if((a->flags&1)&&backend::is_memory(handle))a->size&=4294901759u;
    close_sync(handle,level);if(a->flags&65536)cache::store(a->path,a->buffer,actual,200);return actual;
}
Dword load_into(const char* path,void* buffer,int bytes){Arguments a{path,buffer,Dword(bytes),1,0};return atomic(into_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a));}
Dword try_load_into(const char* path,void* buffer,int bytes){Arguments a{path,buffer,Dword(bytes),0,0};return atomic(into_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a));}
struct ManyArguments {int count;const char*const* paths;void** buffers;Dword flags;int fatal;};
static Dword __cdecl many_atomic(int,Dword context){
    auto* a=reinterpret_cast<ManyArguments*>(context);Dword success=1;
    for(int i=0;i<a->count;++i){a->buffers[i]=try_load(a->paths[i],a->flags);if(!a->buffers[i])success=0;}return success;
}
bool load_many(int count,const char*const* paths,void** buffers,Dword flags){if(count<1)return false;
    ManyArguments a{count,paths,buffers,flags,1};return atomic(many_atomic,path_device(*paths),priority,reinterpret_cast<Dword>(&a))==1;}
bool try_load_many(int count,const char*const* paths,void** buffers,Dword flags){if(count<1)return false;
    ManyArguments a{count,paths,buffers,flags,0};return atomic(many_atomic,path_device(*paths),priority,reinterpret_cast<Dword>(&a))==1;}
static Dword __cdecl directory_atomic(int level,Dword context){
    auto* a=reinterpret_cast<Arguments*>(context);Dword handle;if(!open_sync(a->path,1,level,&handle))return 0;
    auto* result=static_cast<std::uint8_t*>(allocate(a->path,2704,a->size));--level;
    if(!result){close_sync(handle,level);return 0;}read_sync(handle,0,result,2704,level);
    if(big_kind(result)==BigKind::unknown){release(result);close_sync(handle,level);return 0;}
    int bytes=big_directory_size(result);if(bytes>2704){auto* replacement=static_cast<std::uint8_t*>(allocate(a->path,bytes,a->size));
        if(!replacement){release(result);close_sync(handle,level);return 0;}
        kernel::memory_copy(replacement,result,2704);release(result);result=replacement;read_sync(handle,2704,result+2704,bytes-2704,level);}
    close_sync(handle,level);return reinterpret_cast<Dword>(result);
}
std::uint8_t* load_directory(const char* path,Dword flags){Arguments a;a.path=path;a.size=flags;a.fatal=1;
    return reinterpret_cast<std::uint8_t*>(atomic(directory_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a)));}
std::uint8_t* try_load_directory(const char* path,Dword flags){Arguments a;a.path=path;a.size=flags;a.fatal=0;
    return reinterpret_cast<std::uint8_t*>(atomic(directory_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a)));}
static Dword __cdecl save_atomic(int level,Dword context){
    auto* a=reinterpret_cast<Arguments*>(context);Dword handle;if(!open_sync(a->path,6,level,&handle))return 0;
    Dword actual=write_sync(handle,0,a->buffer,a->size,level-1);close_sync(handle,level-1);return actual==a->size;
}
bool save(const char* path,const void* buffer,int bytes){Arguments a;a.path=path;a.buffer=const_cast<void*>(buffer);a.size=bytes;a.fatal=1;
    return atomic(save_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a))==1;}
bool try_save(const char* path,const void* buffer,int bytes){Arguments a;a.path=path;a.buffer=const_cast<void*>(buffer);a.size=bytes;a.fatal=0;
    return atomic(save_atomic,path_device(path),priority,reinterpret_cast<Dword>(&a))==1;}
bool exists(const char* path){return exists_sync(path,priority);}
}
