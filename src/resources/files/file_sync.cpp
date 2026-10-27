#include "nfs/resources/files/filesystem.hpp"
namespace nfs::files {
using Transfer=Dword(__cdecl*)(Dword,int,void*,int,int,Dword);
struct TransferState {int priority;Dword file;int offset;volatile int remaining;int transferred,chunk;std::uint8_t* cursor;Transfer submit;volatile Dword operation;};
static void __cdecl continue_transfer(Dword token,int status,Dword argument){auto* s=reinterpret_cast<TransferState*>(argument);int actual=complete(token);s->operation=0;
    if(status==1){s->offset=int(Dword(s->offset)+Dword(actual));
        auto* next = reinterpret_cast<std::uint8_t*>(reinterpret_cast<Dword>(s->cursor)+Dword(actual));
        s->transferred=int(Dword(s->transferred)+Dword(actual));s->cursor=next;
        if(actual<s->chunk)s->remaining=0;else s->remaining=int(Dword(s->remaining)-Dword(actual));if(s->remaining<1)return;
        s->chunk=s->remaining<8193?s->remaining:8192;s->operation=s->submit(s->file,s->offset,next,s->chunk,s->priority,argument);
        if(s->operation){set_callback(s->operation,continue_transfer);return;}}
    s->remaining=0;
}
static Dword transfer(Dword file,int offset,void* data,int bytes,int priority,Transfer fn){TransferState s{priority,file,offset,bytes,0,bytes,static_cast<std::uint8_t*>(data),fn,0};
    if(bytes>8192)s.chunk=8192;s.operation=fn(file,offset,data,s.chunk,priority,reinterpret_cast<Dword>(&s));
    if(s.operation){set_callback(s.operation,continue_transfer);do{do{wait(s.operation);}while(s.remaining!=0);}while(s.operation!=0);}return s.transferred;
}
bool open_sync(const char* path,Dword mode,int priority,Dword* file){Dword token=open(path,mode,priority,0);if(token){wait(token);int status=poll(token);*file=complete(token);return status==1;}*file=0;return false;}
Dword read_sync(Dword file,int offset,void* data,int bytes,int priority){return transfer(file,offset,data,bytes,priority,read);}
static Dword __cdecl submit_write(Dword f,int o,void* b,int n,int p,Dword a){return write(f,o,b,n,p,a);}
Dword write_sync(Dword file,int offset,const void* data,int bytes,int priority){return transfer(file,offset,const_cast<void*>(data),bytes,priority,submit_write);}
bool close_sync(Dword file,int priority){Dword token=close(file,priority,0);if(token){wait(token);return complete(token)!=0;}return false;}
Dword size_sync(Dword file,int priority){Dword token=size(file,priority,0);if(token){wait(token);return complete(token);}return 0;}
bool exists_sync(const char* path,int priority){Dword token=exists(path,priority,0);if(token){wait(token);return complete(token)!=0;}return false;}
bool erase_sync(const char* path,int priority){Dword token=erase(path,priority,0);if(token){wait(token);return complete(token)!=0;}return false;}
bool truncate_sync(Dword file,int length,int priority){Dword token=truncate(file,length,priority,0);if(token){wait(token);return complete(token)!=0;}return false;}
bool mount_sync(const char* path,Dword pool,int priority,Dword* mount_id){Dword token=mount(path,pool,priority,0);if(token){wait(token);bool success=poll(token)==1;*mount_id=complete(token);return success;}*mount_id=0;return false;}
bool unmount_sync(Dword id,int priority){Dword token=unmount(id,priority,0);if(token){wait(token);return complete(token)!=0;}return false;}
}
