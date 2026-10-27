#include "nfs/resources/files/filesystem.hpp"
namespace nfs::files::async {
static Request* storage=nullptr;static Request* first=nullptr;static Request* last=nullptr;
static int request_count=0;static Dword generation=0;static kernel::Lock pool_lock{};
static Request* resolve(Dword token){if(static_cast<int>(token)>255&&static_cast<int>(token&255)<request_count){auto* r=storage+(token&255);if(r->token==token)return r;}return nullptr;}
static Request* pop(){kernel::lock_enter(&pool_lock);auto* r=first;if(r)first=r->next;kernel::lock_leave(&pool_lock);return r;}
static void push(Request* r){kernel::lock_enter(&pool_lock);if(!first)first=r;else last->next=r;last=r;r->next=nullptr;kernel::lock_leave(&pool_lock);}
static void version(Request* r){generation+=256;if(!generation)generation=256;r->token=(r->token&255)|generation;}
static void recycle(Request* r){if(r->aborted&&r->owned_buffer>1)files::release(reinterpret_cast<void*>(r->owned_buffer));r->token&=255;r->operation=0;push(r);}
static void finish(Request* r){kernel::lock_enter(&pool_lock);Completion fn=r->callback;int aborted=r->aborted,released=r->released;r->operation=0;kernel::lock_leave(&pool_lock);
    if(!aborted){if(fn)fn(r->token);}else if(!released)recycle(r);}
static void __cdecl after_close(Dword,int,Dword context){auto* r=reinterpret_cast<Request*>(context);files::complete(r->operation);finish(r);}
static void __cdecl after_read(Dword,int,Dword context){auto* r=reinterpret_cast<Request*>(context);int actual=files::complete(r->operation);
    const Dword transferred = r->transferred + Dword(actual);
    auto* cursor = reinterpret_cast<std::uint8_t*>(reinterpret_cast<Dword>(r->cursor) + Dword(actual));
    r->transferred = transferred; r->cursor = cursor;
    if(actual<8192||r->aborted){r->operation=files::close(r->file,99,context);
        if(!r->operation){kernel::error("ASYNCFILE_load[at] - unable to initiate file close\n");return;}files::set_callback(r->operation,after_close);return;}
    const Dword offset = r->offset + Dword(actual);
    int count = int(Dword(r->remaining) - Dword(actual));
    r->offset = offset; r->remaining = count; if(count>8192)count=8192;
    r->operation=files::read(r->file,offset,cursor,count,99,context);
    if(!r->operation){kernel::error("ASYNCFILE_load[at] - unable to initiate file read\n");return;}files::set_callback(r->operation,after_read);
}
static void __cdecl after_size(Dword,int,Dword context){auto* r=reinterpret_cast<Request*>(context);Dword bytes=files::complete(r->operation);
    if(!r->aborted){Dword pool=r->remaining;r->remaining=2147483647;void* buffer=files::allocate("ASYNCFILEBUF",bytes,pool);
        r->owned_buffer=reinterpret_cast<Dword>(buffer);r->cursor=static_cast<std::uint8_t*>(buffer);
        r->operation=files::read(r->file,r->offset,buffer,8192,99,context);
        if(!r->operation){kernel::error("ASYNCFILE_load - unable to initiate file read\n");return;}files::set_callback(r->operation,after_read);return;}
    r->operation=files::close(r->file,99,context);if(!r->operation){kernel::error("ASYNCFILE_load - unable to initiate file close\n");return;}files::set_callback(r->operation,after_close);
}
static void __cdecl after_open(Dword,int,Dword context){auto* r=reinterpret_cast<Request*>(context);r->file=files::complete(r->operation);
    if(!r->file){finish(r);return;}
    if(r->aborted){r->operation=files::close(r->file,99,context);if(!r->operation){kernel::error("ASYNCFILE_load[at] - unable to initiate file close\n");return;}files::set_callback(r->operation,after_close);return;}
    if(r->owned_buffer){r->operation=files::size(r->file,99,context);if(!r->operation){kernel::error("ASYNCFILE_load - unable to initiate file size get\n");return;}files::set_callback(r->operation,after_size);return;}
    int count=r->remaining;if(count>8192)count=8192;r->operation=files::read(r->file,r->offset,r->cursor,count,99,context);
    if(!r->operation){kernel::error("ASYNCFILE_loadat - unable to initiate file read\n");return;}files::set_callback(r->operation,after_read);
}
static void __cdecl after_handle_read(Dword,int,Dword context){auto* r=reinterpret_cast<Request*>(context);int actual=files::complete(r->operation);
    const Dword transferred = r->transferred + Dword(actual);
    auto* cursor = reinterpret_cast<std::uint8_t*>(reinterpret_cast<Dword>(r->cursor) + Dword(actual));
    r->transferred = transferred; r->cursor = cursor;if(actual<8192||r->aborted){finish(r);return;}
    const Dword offset = r->offset + Dword(actual);
    int count = int(Dword(r->remaining) - Dword(actual));
    r->offset = offset; r->remaining = count; if(count>8192)count=8192;
    r->operation=files::read(r->file,offset,cursor,count,99,context);
    if(!r->operation){kernel::error("ASYNCFILE_read - unable to initiate file read\n");return;}files::set_callback(r->operation,after_handle_read);
}
void initialize(int count,Dword pool){if(storage){kernel::error("ASYNCFILE_init - already initialized\n");return;}
    if(count<257){request_count=count;first=static_cast<Request*>(files::allocate("ASYNCFILE",Dword(count)*48,pool));last=first+count-1;storage=first;kernel::lock_init(&pool_lock);
        for(int i=0;i<count;++i){storage[i].token=i;storage[i].operation=0;storage[i].next=storage+i+1;}storage[count-1].next=nullptr;return;}
    kernel::error("ASYNCFILE_init - too many async requests specified (max 256)\n");
}
Dword load(const char* path,Dword pool){auto* r=pop();if(!r){kernel::error("ASYNCFILE_load - out of async request blocks\n");return 0;}version(r);
    r->transferred=0;r->released=r->aborted=0;r->owned_buffer=1;r->callback=nullptr;r->offset=0;r->remaining=pool;r->cursor=nullptr;
    r->operation=files::open(path,1,100,reinterpret_cast<Dword>(r));if(!r->operation){kernel::error("ASYNCFILE_load - unable to initiate file open\n");return 0;}
    files::set_callback(r->operation,after_open);return r->token;
}
Dword load_into(const char* path,void* buffer,int count){auto* r=pop();if(!r){kernel::error("ASYNCFILE_loadat - out of async request blocks\n");return 0;}version(r);
    r->transferred=0;r->released=r->aborted=0;r->owned_buffer=0;r->callback=nullptr;r->offset=0;r->remaining=count<1?2147483647:count;r->cursor=static_cast<std::uint8_t*>(buffer);
    r->operation=files::open(path,1,100,reinterpret_cast<Dword>(r));if(!r->operation){kernel::error("ASYNCFILE_loadat - unable to initiate file open\n");return 0;}files::set_callback(r->operation,after_open);return r->token;
}
Dword read(Dword file,Dword offset,void* buffer,int count){auto* r=pop();if(!r){kernel::error("ASYNCFILE_read - out of async request blocks\n");return 0;}version(r);
    r->transferred=0;r->released=r->aborted=0;r->owned_buffer=0;r->callback=nullptr;r->file=file;r->offset=offset;r->remaining=count;r->cursor=static_cast<std::uint8_t*>(buffer);
    if(count>8192)count=8192;r->operation=files::read(file,offset,buffer,count,100,reinterpret_cast<Dword>(r));
    if(!r->operation){kernel::error("ASYNCFILE_read - unable to initiate file read\n");return 0;}files::set_callback(r->operation,after_handle_read);return r->token;
}
void set_callback(Dword token,Completion fn){auto* r=resolve(token);if(!r){kernel::error("ASYNCFILE_setcallback - unknown request handle\n");return;}
    if(r->callback){kernel::error("ASYNCFILE_setcallback - callback already installed\n");return;}
    kernel::lock_enter(&pool_lock);Dword operation=r->operation;r->callback=fn;kernel::lock_leave(&pool_lock);if(!operation)fn(token);
}
int poll(Dword token){kernel::lock_enter(&pool_lock);auto* r=resolve(token);if(!r){kernel::lock_leave(&pool_lock);return -1;}Dword operation=r->operation;kernel::lock_leave(&pool_lock);return operation==0;}
int cancel(Dword token){int prior_aborted=0,prior_released=0;Dword operation=0;kernel::lock_enter(&pool_lock);auto* r=resolve(token);
    if(r){operation=r->operation;prior_aborted=r->aborted;prior_released=r->released;if(operation||!prior_released)r->aborted=1;}
    kernel::lock_leave(&pool_lock);if(r&&!prior_aborted&&r->aborted){if(operation){files::cancel(operation);return 1;}if(!prior_released)recycle(r);return 1;}return -1;
}
int take(Dword token,void** buffer,Dword* transferred){int aborted=0,released=0;kernel::lock_enter(&pool_lock);auto* r=resolve(token);
    if(r){aborted=r->aborted;released=r->released;if(!aborted)r->released=1;}kernel::lock_leave(&pool_lock);
    if(r&&!aborted){if(released)kernel::error("ASYNCFILE_release - release called more than once for this request\n");while(r->operation)files::wait(r->operation);
        if(!r->aborted){if(buffer)*buffer=reinterpret_cast<void*>(reinterpret_cast<Dword>(r->cursor)-r->transferred);if(transferred)*transferred=r->transferred;recycle(r);return 1;}
        if(buffer)*buffer=nullptr;if(transferred)*transferred=0;recycle(r);}
    return -1;
}
void shutdown(){if(!storage)return;for(int i=0;i<request_count;++i)cancel(storage[i].token);
    for(;;){bool active=false;for(int i=0;i<request_count;++i)if(storage[i].operation)active=true;if(!active)break;
        if(kernel::is_thread(nullptr))kernel::cooperate(0);kernel::thread_yield(0);}
    files::release(storage);kernel::lock_destroy(&pool_lock);storage=nullptr;
}
}
