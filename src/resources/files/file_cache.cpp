#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/archives/big_directory.hpp"
#include "nfs/core/console/console.hpp"
#include <cstdio>
#include <cstring>
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::files::cache {
Settings settings{40,nullptr,nullptr,nullptr,0,nullptr,0,0};
Header* database=nullptr;Dword enabled=0;
static Header* current_database() {
    Header* volatile& table = database;
    return table;
}
volatile std::uint8_t busy=0,stopping=0,pausing=0,initialized=0;
std::uint8_t dirty=0;kernel::Lock lock{};kernel::Queue queue{};kernel::Signal signal{};kernel::Thread thread{};
int __cdecl compare_entry(const void* key,const void* row){return big_name_compare(static_cast<const char*>(key),static_cast<const char*>(row));}
std::uint8_t get_settings(Settings* result){Dword size=result->size;if(static_cast<int>(size)>40)kernel::memory_zero(result,size);kernel::memory_copy(result,&settings,size);return 1;}
std::uint8_t set_settings(const Settings* input){if(static_cast<int>(input->size)<40)kernel::memory_zero(&settings,40);kernel::memory_copy(&settings,input,input->size);
    if(settings.destination){std::size_t length=std::strlen(settings.destination);char c=*settings.destination;
        if(c>='a'&&c<='z')*settings.destination=c-32;if(settings.destination[length-1]!='\\')std::strcat(settings.destination,"\\");
        if(settings.size==40){c=*settings.source;if(c>='a'&&c<='z')*settings.source=c-32;}}
    return 1;
}
static Dword __cdecl queued_key(const void* value,Dword){return static_cast<const Queued*>(value)->input.order;}
static void __cdecl worker(){
    kernel::signal_wait(&signal);for(;;){if(stopping){busy=1;flush();return;}busy=1;
        while(!pausing){auto* item=static_cast<Queued*>(kernel::queue_pop(&queue));if(!item)break;store(item->input.name,nullptr,0,item->input.priority);release(item);}
        flush();pausing=0;busy=0;if(stopping)break;kernel::signal_wait(&signal);}
}
void initialize(){
    initialized=1;pausing=0;busy=0;stopping=0;dirty=0;kernel::lock_init(&lock);kernel::queue_init(&queue,queued_key,0);
    kernel::signal_init(&signal);kernel::thread_start_no_argument(&thread,worker,0,0,-3);enabled=1;
    char path[256];std::strcpy(path,settings.destination);std::strcat(path,"cache.dat");database=static_cast<Header*>(file::try_load(path,0));
    if(database){if(database->size==296&&database->version>0&&database->version<2&&
            database->checksum==kernel::checksum32(reinterpret_cast<const std::uint8_t*>(database+1),Dword(database->count)*261))return;
        release(database);database=nullptr;}
}
int state(){return (busy!=0)+1;}
void pause(){pausing=1;}
void start(){busy=1;kernel::signal_set(&signal);}
void shutdown(){
    stopping=1;if(state()==2)pause();else kernel::signal_set(&signal);
    while(!kernel::thread_finished(&thread)){if(kernel::is_thread(nullptr))kernel::cooperate(0);kernel::thread_yield(10);}
    kernel::thread_close(&thread);kernel::signal_destroy(&signal);auto* item=static_cast<Queued*>(kernel::queue_pop(&queue));
    while(item){release(item);item=static_cast<Queued*>(kernel::queue_pop(&queue));}
    kernel::queue_destroy(&queue);kernel::lock_destroy(&lock);initialized=0;enabled=0;flush();release(database);database=nullptr;
}
void flush() {
    if (dirty != 1)
        return;
    Header* header = current_database();
    const Dword bytes = Dword(header->count) * 261;
    header->size = 296;
    current_database()->version = 1;
    const Dword checksum = kernel::checksum32(
        reinterpret_cast<const std::uint8_t*>(current_database() + 1), bytes);
    current_database()->checksum = checksum;
    char path[260];
    std::strcpy(path, settings.destination);
    std::strcat(path, "cache.dat");
    file::save(path, current_database(), int(bytes + 296));
    dirty = 0;
}
Entry* insert(const char* name) {
    int count = 1;
    kernel::lock_enter(&lock);
    Header* previous = current_database();
    if (previous)
        count = int(Dword(previous->count) + 1);
    const Dword bytes = Dword(count) * 261 + 560;
    Header* allocated = static_cast<Header*>(allocate("FCache Info", int(bytes), 0));
    database = allocated;
    kernel::memory_zero(allocated, bytes);
    current_database()->count = count;
    if (previous) {
        const volatile Dword* previous_bytes = reinterpret_cast<const volatile Dword*>(&previous->bytes);
        volatile Dword* low = reinterpret_cast<volatile Dword*>(&current_database()->bytes);
        low[0] = previous_bytes[0];
        volatile Dword* high = reinterpret_cast<volatile Dword*>(&current_database()->bytes);
        high[1] = previous_bytes[1];
    }
    Header* header = current_database();
    auto* old = reinterpret_cast<Entry*>(reinterpret_cast<Dword>(previous) + 296);
    auto* output = reinterpret_cast<Entry*>(header + 1);
    int copied = 0;
    int old_count = int(Dword(header->count) - 1);
    while (copied < old_count) {
        if (big_name_compare(name, old->name) <= 0)
            break;
        kernel::memory_copy(output, old, 261);
        output = reinterpret_cast<Entry*>(reinterpret_cast<Dword>(output) + 261);
        old = reinterpret_cast<Entry*>(reinterpret_cast<Dword>(old) + 261);
        copied = int(Dword(copied) + 1);
        old_count = int(Dword(current_database()->count) - 1);
    }
    std::strcpy(output->name, name);
    output->cached = 0;
    const Dword remaining = (Dword(current_database()->count) - Dword(copied) - 1) * 261;
    kernel::memory_copy(output + 1, old, remaining);
    if (previous)
        release(previous);
    kernel::lock_leave(&lock);
    dirty = 1;
    return output;
}
void enqueue(const Input* entries,int count){
    if(count){do{if(!backend::hint(entries->name)){char path[260];std::strcpy(path,settings.destination);std::strcat(path,entries->name);
            HANDLE handle=CreateFileA(path,GENERIC_READ,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
            if(handle==INVALID_HANDLE_VALUE){auto* item=static_cast<Queued*>(allocate("queued file",272,0));kernel::memory_copy(&item->input,entries,268);kernel::queue_insert(&queue,item);}
            else{kernel::print::write(7,"Already in cache, updating database.\n");Entry* entry=insert(entries->name);database->bytes+=GetFileSize(handle,nullptr);entry->cached=1;CloseHandle(handle);}}
        ++entries;count=int(Dword(count)-1);}while(count);}
    start();
}
int enqueue_list(const char* path){
    Queued* item=nullptr;int queued=0;Dword bytes;auto* data=static_cast<char*>(file::try_load_with_length(path,&bytes,0));if(!data)return 0;char* cursor=data;
    if(static_cast<int>(bytes)>0){do{if(!item)item=static_cast<Queued*>(allocate("queued file",272,0));int unused;
            std::sscanf(cursor,"%d,%d,%s",&unused,&item->input.order,item->input.name);cursor=std::strchr(cursor,'\n')+1;
            if(!backend::hint(item->input.name)){char full[260];std::strcpy(full,settings.destination);std::strcat(full,item->input.name);
                HANDLE handle=CreateFileA(full,GENERIC_READ,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
                if(handle==INVALID_HANDLE_VALUE){if(static_cast<int>(item->input.order)>1){item->input.priority=200;kernel::queue_insert(&queue,item);item=nullptr;queued=int(Dword(queued)+1);}}
                else{kernel::print::write(7,"Already in cache, updating database.\n");Entry* entry=insert(item->input.name);database->bytes+=GetFileSize(handle,nullptr);entry->cached=1;CloseHandle(handle);}}
        }while(static_cast<int>(reinterpret_cast<Dword>(cursor)-reinterpret_cast<Dword>(data))<static_cast<int>(bytes));
        if(item)release(item);}
    release(data);start();return queued;
}
Dword account_existing(const char* name){
    Entry* entry=nullptr;if(database)entry=static_cast<Entry*>(kernel::binary_search(name,database+1,database->count,261,compare_entry));if(!entry)insert(name);
    char path[260];std::strcpy(path,settings.destination);std::strcat(path,name);HANDLE handle=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(handle==INVALID_HANDLE_VALUE)return 0;kernel::lock_enter(&lock);database->bytes+=GetFileSize(handle,nullptr);kernel::lock_leave(&lock);CloseHandle(handle);return 1;
}
void report_error(Dword message){
    if(!message)message=GetLastError();char* text=reinterpret_cast<char*>(message);
    FormatMessageA(4864,nullptr,message,1024,reinterpret_cast<char*>(&text),0,nullptr);OutputDebugStringA(text);LocalFree(text);
}
void create_directories(const char* filename){
    char path[260];std::strcpy(path,filename);char* separator=std::strchr(path,'\\');if(!separator)separator=std::strchr(path,'/');
    while(separator){do{*separator=0;if(!CreateDirectoryA(path,nullptr)){Dword error=GetLastError();if(error!=ERROR_ALREADY_EXISTS)report_error(error);}
            *separator='\\';separator=std::strchr(separator+1,'\\');}while(separator);separator=std::strchr(path,'/');}
}
}
