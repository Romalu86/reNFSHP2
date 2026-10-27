#include "nfs/resources/files/filesystem.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/archives/big_directory.hpp"
#include <cstring>
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::files {
Allocate& allocate=cache::settings.allocator;Release& release=cache::settings.deallocator;
void set_memory(Allocate a,Release f){allocate=a;release=f;}
namespace detail {
Device* devices=nullptr;kernel::Queue free_requests{},mounts{};volatile int stop=0;
static kernel::Lock mount_lock{};static Dword mount_token=~Dword(0);
static Dword __cdecl request_key(const void* p,Dword){auto* r=static_cast<const Request*>(p);return ((r->token>>5)&16777215)|(Dword(r->priority)<<24);}
int compare_token(void* p,Dword token){return static_cast<Request*>(p)->token==token;}
Request* find(Device* d,Dword token,int* phase){*phase=0;if(d->active){if(d->current&&d->current->token==token){*phase=-1;return d->current;}
    auto* r=static_cast<Request*>(kernel::queue_find(&d->pending,compare_token,token));if(r){*phase=1;return r;}
    return static_cast<Request*>(kernel::queue_find(&d->completed,compare_token,token));}return nullptr;}
static void __cdecl worker(Dword device){auto* d=devices+device;d->active=1;SetEvent(d->completion);
    for(;;){Request* r;for(;;){if(stop){d->active=0;return;}if(d->active)kernel::lock_enter(&d->pending.lock);
        r=static_cast<Request*>(kernel::queue_pop(&d->pending));d->current=r;
        if(r&&d->atomic_priority<r->priority){kernel::queue_insert(&d->pending,r);d->current=nullptr;r=nullptr;}
        if(d->active)kernel::lock_leave(&d->pending.lock);if(d->current)break;kernel::signal_wait(&d->wake);}
        if(!(r->flags&2)){
            auto fail=[&](){r->status=-2;r->win32_error=GetLastError();};
            switch(r->operation){
            case 0:r->file=open_backend_or_mount(reinterpret_cast<const char*>(r->value_2c),r->value_24,(r->value_24&1)!=0);if(!r->file)fail();else r->status=1;break;
            case 2:case 3:
                if(backend::seek(r->file,r->value_24)){
                    r->value_28=r->operation==2?backend::read(r->file,reinterpret_cast<void*>(r->value_2c),r->value_28):
                        backend::write(r->file,reinterpret_cast<const void*>(r->value_2c),r->value_28);r->status=1;
                }r->win32_error=GetLastError();if(r->win32_error)r->status=-2;break;
            case 4:if(!backend::information(r->file,nullptr,&r->value_24)){r->value_24=0;fail();}else r->status=1;break;
            case 5:r->status=backend::truncate(r->file,r->value_28)?1:-2;if(r->status==-2)fail();break;
            case 6:r->file=open_backend_or_mount(reinterpret_cast<const char*>(r->value_2c),1,true);if(!r->file){r->value_24=0;r->status=1;}
                else {backend::close(r->file);r->value_24=1;r->status=1;}break;
            case 7:if(!backend::erase(reinterpret_cast<const char*>(r->value_2c))){fail();break;}[[fallthrough]];
            case 1:r->status=1;break;
            case 8:r->status=1;r->win32_error=0;break;
            case 9:{auto* m=reinterpret_cast<Mount*>(r->value_2c);if(m){m->file=open_backend_or_mount(m->name,9,true);Dword flags,length;
                backend::information(m->file,&flags,&length);if(!m->file||!(flags&8))fail();else {r->status=1;m->data=static_cast<std::uint8_t*>(backend::data(m->file));}}break;}
            case 10:kernel::queue_remove(&mounts,reinterpret_cast<Mount*>(r->value_2c));r->status=1;break;
            default:kernel::error("FILE_devicethread - UNKNOWN TYPE OF OPERATION %d.\n",d->current->operation);break;
            }
        }
        if(d->active)kernel::lock_enter(&d->pending.lock);kernel::queue_push_front(&d->completed,r);d->current=nullptr;
        if(d->active)kernel::lock_leave(&d->pending.lock);
        if(r->callback){Dword flags=r->flags;r->flags=flags|8;int status=(flags&2)?-1:r->status;r->callback(r->token,status,r->argument);}
        SetEvent(d->completion);
    }
}
void start_device(int device){auto* d=devices+device;if(d->active)return;kernel::lock_enter(&free_requests.lock);
    d->current=nullptr;kernel::queue_init(&d->pending,request_key,0);kernel::queue_clone_lock(&d->completed,nullptr,0,&d->pending);
    kernel::signal_init(&d->wake);d->completion=CreateEventA(nullptr,TRUE,FALSE,nullptr);kernel::lock_init(&d->atomic_lock);d->serial=1;d->atomic_priority=255;
    if(kernel::thread_start(&d->thread,worker,device,0,0,1)){WaitForSingleObject(d->completion,INFINITE);ResetEvent(d->completion);kernel::lock_leave(&free_requests.lock);return;}
    kernel::error("FILE_startdevice - FAILED TO START THREAD FOR FILE SYSTEM DEVICE.\n");kernel::lock_leave(&free_requests.lock);
}
Request* reserve(Dword operation,std::uint8_t priority,Dword argument,Dword device){auto* r=static_cast<Request*>(kernel::queue_pop(&free_requests));auto* d=devices+device;
    if(!r){kernel::error("FILE_allocateop - NO FREE OPS LEFT TO ALLOCATE.\n");return nullptr;}if(!d->active)start_device(device);
    r->priority=priority;r->operation=operation;r->status=0;r->flags&=4294967280u;r->win32_error=0;r->file=0;r->argument=argument;r->callback=nullptr;
    r->value_24=r->value_28=r->value_2c=0;if(d->active)kernel::lock_enter(&d->pending.lock);
    r->token=(d->serial<<5)|device;d->serial=(d->serial+1)&16777215;if(!d->serial)d->serial=1;
    if(d->active)kernel::lock_leave(&d->pending.lock);return r;
}
Dword enqueue(Request* r){auto* d=devices+(r->token&31);if(!d)kernel::error("FILE_queueop - ATTEMPT TO QUEUE FILEOP ON NON-EXISTANT DEVICE.\n");
    kernel::queue_insert(&d->pending,r);kernel::signal_set(&d->wake);return r->token;}
void copy_path(Request* r,const char* path){if(!path)path="";Dword size=static_cast<Dword>(std::strlen(path)+1);r->flags|=1;
    void* copy=allocate(path,size,256);r->value_2c=reinterpret_cast<Dword>(copy);kernel::memory_copy(copy,path,size);}
struct OpenContext {char* archive;const char* member;Dword handle;int success;};
static int __cdecl open_member(void* pointer,Dword argument){auto* m=static_cast<Mount*>(pointer);auto* c=reinterpret_cast<OpenContext*>(argument);
    unsigned char first=static_cast<unsigned char>(*c->archive);if(first&&std::strcmp(m->name,c->archive))return 1;
    Dword offset,length;const char* member=big_find_entry(m->data,c->member,0,&offset,&length);
    if(member){c->success=backend::memory_file(m->data+offset,length,static_cast<std::uint8_t>(m->device),&c->handle);return 0;}return first==0;
}
Dword open_backend_or_mount(const char* path,Dword flags,bool fallback){char archive[260];OpenContext c{archive,path,0,0};const char* separator=std::strchr(path,'|');
    if(!separator){c.success=backend::open(path,flags,&c.handle);archive[0]=0;}else {std::strncpy(archive,path,separator-path);archive[separator-path]=0;c.member=separator+1;}
    if(fallback&&!c.success)kernel::queue_visit_locked(&mounts,open_member,reinterpret_cast<Dword>(&c));return c.handle;
}
}
using namespace detail;
int storage_size(int handles,int,int operations){if(!handles)handles=64;if(!operations)operations=64;return (handles+172)*32+operations*48;}
Dword initialize(int handles,int unused,int operations){void* storage=allocate("File System",storage_size(handles,0,operations),256);return initialize_in(storage,handles,unused,operations);}
Dword initialize_in(void* storage,int handles,int,int operations){if(devices)return 1;kernel::thread_initialize();if(!handles)handles=64;if(!operations)operations=64;
    stop=0;kernel::queue_init(&free_requests,nullptr,0);kernel::queue_init(&mounts,nullptr,0);devices=static_cast<Device*>(storage);kernel::memory_zero(storage,5504);
    auto* requests=reinterpret_cast<Request*>(static_cast<char*>(storage)+5504);kernel::memory_zero(requests,operations*48);
    for(int i=0;i<operations;++i)kernel::queue_push_front(&free_requests,requests+i);
    kernel::lock_init(&mount_lock);mount_token=~Dword(0);backend::initialize(handles,requests+operations);kernel::register_exit(shutdown);return 1;
}
void shutdown(){if(!devices)return;auto* m=static_cast<Mount*>(kernel::queue_pop(&mounts));while(m){backend::close(m->file);release(m);m=static_cast<Mount*>(kernel::queue_pop(&mounts));}
    stop=1;for(int i=0;i<32;++i){auto* d=devices+i;if(d->active){kernel::signal_set(&d->wake);while(d->active)kernel::thread_yield(0);
        kernel::signal_destroy(&d->wake);SetEvent(d->completion);CloseHandle(d->completion);kernel::queue_destroy(&d->completed);kernel::queue_destroy(&d->pending);kernel::lock_destroy(&d->atomic_lock);}}
    backend::shutdown();kernel::queue_destroy(&free_requests);kernel::queue_destroy(&mounts);release(devices);kernel::lock_destroy(&mount_lock);devices=nullptr;
}
int poll(Dword token){auto* d=devices+(token&31);if(d->active)kernel::lock_enter(&d->pending.lock);int phase;Request* r=find(d,token,&phase);
    int result=phase?0:(!r?-3:(r->flags&2)?-1:r->status);if(d->active)kernel::lock_leave(&d->pending.lock);return result;
}
int wait(Dword token){auto* d=devices+(token&31);if(!token||!d->active)return -3;
    for(;;){if(d->active)kernel::lock_enter(&d->pending.lock);int phase;auto* r=find(d,token,&phase);bool pending=(phase!=0)&&r;
        if(d->active)kernel::lock_leave(&d->pending.lock);if(!pending)break;
        if(!kernel::is_thread(nullptr)){WaitForSingleObject(d->completion,INFINITE);ResetEvent(d->completion);}else {kernel::cooperate(0);kernel::thread_yield(1);}}
    return poll(token);
}
Dword complete(Dword token){auto* d=devices+(token&31);auto* r=static_cast<Request*>(kernel::queue_extract(&d->completed,compare_token,token));
    if(!d||!r)kernel::error("FILESYS_completeop - OPERATION NOT FOUND IN COMPLETED LIST.\n");Dword result=0;
    switch(r->operation){
    case 0:if(!(r->flags&2)||!r->file)result=r->file;else backend::close(r->file);break;
    case 1:result=backend::close(r->file);break;
    case 2:case 3:result=r->value_28;break;
    case 4:case 6:result=r->value_24;break;
    case 8:break;
    case 9:{auto* m=reinterpret_cast<Mount*>(r->value_2c);if(m){if(!(r->flags&2)&&r->status==1){kernel::queue_push_back(&mounts,m);result=m->token;}
        else {if(m->file)backend::close(m->file);release(m);}}break;}
    case 10:{auto* m=reinterpret_cast<Mount*>(r->value_2c);if(m){backend::close(m->file);release(m);result=1;}break;}
    default:result=r->status==1;break;
    }
    if(r->flags&1)release(reinterpret_cast<void*>(r->value_2c));r->token=0;kernel::queue_push_back(&free_requests,r);return result;
}
Dword context(Dword token){auto* d=devices+(token&31);if(d->active)kernel::lock_enter(&d->pending.lock);int phase;auto* r=find(d,token,&phase);Dword value=r?r->argument:0;if(d->active)kernel::lock_leave(&d->pending.lock);return value;}
Dword operation_device(Dword token){auto* d=devices+(token&31);if(d->active)kernel::lock_enter(&d->pending.lock);int phase;auto* r=find(d,token,&phase);
    if(d->active)kernel::lock_leave(&d->pending.lock);return r?(token&31):0;}
void set_callback(Dword token,Callback callback){auto* d=devices+(token&31);if(!callback)kernel::error("FILESYS_callbackop - can not specify a NULL callback.\n");
    if(d->active)kernel::lock_enter(&d->pending.lock);int phase;auto* r=find(d,token,&phase);
    if(!r)kernel::error("FILESYS_callbackop - UNKNOWN FILEOP.\n");else {Dword flags=r->flags;r->flags=flags|4;
        if(!phase){r->callback=nullptr;r->flags=flags|12;callback(r->token,r->status,r->argument);}else r->callback=callback;}
    if(d->active)kernel::lock_leave(&d->pending.lock);
}
void set_priority(Dword token,std::uint8_t priority){auto* d=devices+(token&31);if(d){auto* r=static_cast<Request*>(kernel::queue_extract(&d->pending,compare_token,token));if(r){r->priority=priority;enqueue(r);}}}
void cancel(Dword token){auto* d=devices+(token&31);if(d->active)kernel::lock_enter(&d->pending.lock);int phase;auto* r=find(d,token,&phase);
    if(r&&phase){switch(r->operation){case 2:case 3:if(d->current==r)backend::cancel_read(r->file);[[fallthrough]];
        case 0:case 4:case 6:case 8:case 9:r->flags|=2;if(kernel::queue_remove(&d->pending,r)){kernel::queue_push_front(&d->completed,r);
            if(r->callback){r->flags|=8;r->callback(r->token,-1,r->argument);}}break;default:break;}}
    if(d->active)kernel::lock_leave(&d->pending.lock);
}
int path_device(const char* path){if(!std::strchr(path,'|')){int drive=backend::path_drive(path);return drive?drive-64:0;}return 0;}
Dword open(const char* path,Dword mode,int priority,Dword arg){auto* r=reserve(0,static_cast<std::uint8_t>(priority),arg,path_device(path));copy_path(r,path);r->value_24=mode;return enqueue(r);}
Dword exists(const char* path,int priority,Dword arg){auto* r=reserve(6,static_cast<std::uint8_t>(priority),arg,path_device(path));copy_path(r,path);return enqueue(r);}
Dword erase(const char* path,int priority,Dword arg){auto* r=reserve(7,static_cast<std::uint8_t>(priority),arg,path_device(path));copy_path(r,path);return enqueue(r);}
Dword close(Dword file,int priority,Dword arg){auto* r=reserve(1,static_cast<std::uint8_t>(priority),arg,backend::device(file));r->file=file;return enqueue(r);}
Dword read(Dword file,int offset,void* buffer,int bytes,int priority,Dword arg){auto* r=reserve(2,static_cast<std::uint8_t>(priority),arg,backend::device(file));r->value_28=bytes;r->file=file;r->value_24=offset;r->value_2c=reinterpret_cast<Dword>(buffer);return enqueue(r);}
Dword write(Dword file,int offset,const void* buffer,int bytes,int priority,Dword arg){auto* r=reserve(3,static_cast<std::uint8_t>(priority),arg,backend::device(file));r->value_28=bytes;r->file=file;r->value_24=offset;r->value_2c=reinterpret_cast<Dword>(buffer);return enqueue(r);}
Dword size(Dword file,int priority,Dword arg){auto* r=reserve(4,static_cast<std::uint8_t>(priority),arg,backend::device(file));r->file=file;return enqueue(r);}
Dword truncate(Dword file,int position,int priority,Dword arg){auto* r=reserve(5,static_cast<std::uint8_t>(priority),arg,backend::device(file));r->file=file;r->value_28=position;return enqueue(r);}
Dword mount(const char* path,Dword pool,int priority,Dword arg){auto* m=static_cast<Mount*>(allocate("bigfile",280,pool));if(!m)return 0;Dword device=path_device(path);
    kernel::lock_enter(&mount_lock);m->token=mount_token--;kernel::lock_leave(&mount_lock);m->device=device;m->file=0;m->data=nullptr;std::strcpy(m->name,path);
    auto* r=reserve(9,static_cast<std::uint8_t>(priority),arg,device);r->value_2c=reinterpret_cast<Dword>(m);return enqueue(r);}
static int __cdecl mount_match(void* p,Dword token){return static_cast<Mount*>(p)->token==token;}
Dword unmount(Dword token,int priority,Dword arg){auto* m=static_cast<Mount*>(kernel::queue_find(&mounts,mount_match,token));if(!m)return 0;
    auto* r=reserve(10,static_cast<std::uint8_t>(priority),arg,backend::device(m->file));r->value_2c=reinterpret_cast<Dword>(m);return enqueue(r);}
Dword atomic(Atomic fn,int device,int priority,Dword arg){if(!devices)kernel::error("FILESYS_atomic - FILE SYSTEM NOT INITIALIZED, CALL FILESYS_init().\n");
    if(device<0||device>31){kernel::error("FILESYS_atomic - CALLED WITH ILLEGAL FILE DEVICE (%d).\n",device);return 0;}
    auto* d=devices+device;if(!d->active)start_device(device);kernel::lock_enter(&d->atomic_lock);int previous=d->atomic_priority;
    if(priority<=previous){d->atomic_priority=priority;Dword result=fn(priority,arg);d->atomic_priority=previous;kernel::signal_set(&d->wake);kernel::lock_leave(&d->atomic_lock);return result;}
    kernel::error("FILESYS_atomic - CALLED AT PRIORITY (%d) LOWER THAN CURRENT DEVICE PRIORITY (%d).\n",priority,previous);kernel::lock_leave(&d->atomic_lock);return 0;
}
void set_directory(const char* path){SetLastError(0);if(!SetCurrentDirectoryA(path))kernel::error("setdirectory - FAILED ON '%s'\n",path);}
void get_directory(char* path){SetLastError(0);DWORD n=GetCurrentDirectoryA(511,path);if(!n||n>511)kernel::error("getdirectory - FAILED TO FETCH, OR WAS TOO LONG (%D CHAR).\n",n);}
}
