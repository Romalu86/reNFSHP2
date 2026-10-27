#include "nfs/resources/files/file_cache.hpp"
#include "nfs/core/console/console.hpp"
#include <cstring>
namespace nfs::files::cache {
std::uint8_t store(const char* name,const void* data,Dword bytes,int priority){
    kernel::print::write(7,"Caching file %s... ",name);if(backend::hint(name)){kernel::print::write(7,"Already cached.\n");return 1;}
    if(!data)return copy(name,priority);std::uint64_t available=0;if(settings.free_space_limit>=0)available=backend::free_space(settings.destination);
    std::int64_t signed_bytes=static_cast<std::int32_t>(bytes);std::uint64_t used=database?database->bytes:0;
    if(static_cast<std::int64_t>(used+signed_bytes)>=settings.byte_limit)return 0;

    // The stream branch has the opposite comparison; this is in the artifact.
    if(static_cast<std::int64_t>(available-signed_bytes)>settings.free_space_limit)return 0;
    char path[260];std::strcpy(path,settings.destination);std::strcat(path,name);create_directories(path);
    HANDLE handle=CreateFileA(path,GENERIC_WRITE,0,nullptr,TRUNCATE_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(handle==INVALID_HANDLE_VALUE){kernel::print::write(7,"CacheDataFile - first open attempt failed on file %s.\n",name);
        handle=CreateFileA(path,GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(handle==INVALID_HANDLE_VALUE)return 0;}
    kernel::print::write(7,"CacheDataFile - Writing file %s to disk cache.\n",name);Entry* entry=insert(name);DWORD written;
    WriteFile(handle,data,bytes,&written,nullptr);CloseHandle(handle);if(written!=bytes){DeleteFileA(path);return 0;}
    kernel::lock_enter(&lock);std::uint64_t total=database->bytes+signed_bytes;
    if(static_cast<std::int64_t>(total)<settings.byte_limit){database->bytes=total;entry->cached=1;kernel::lock_leave(&lock);return 1;}
    if(!DeleteFileA(path))report_error(0);kernel::lock_leave(&lock);return 0;
}
Dword stream(Dword file,HANDLE output,int priority,std::uint16_t* checksum){
    std::uint8_t buffers[2][8192];Dword operations[2];DWORD written;Dword position=SetFilePointer(output,0,nullptr,FILE_CURRENT);
    operations[0]=read(file,position,buffers[0],8192,priority,0);Dword current=0;
    for(;;){Dword next=current^1;operations[next]=read(file,position+8192,buffers[next],8192,priority,0);
        wait(operations[current]);Dword bytes=complete(operations[current]);WriteFile(output,buffers[current],bytes,&written,nullptr);
        if(GetLastError()){report_error(0);position=~Dword(0);break;}
        *checksum=static_cast<std::uint16_t>(kernel::checksum16(buffers[current],bytes,*checksum));position+=bytes;current=next;
        if(bytes!=8192||stopping)break;
    }
    // Error and normal exits use the same original index, even when the current
    // operation was already completed. No replacement cleanup policy is inserted.
    wait(operations[current]);complete(operations[current]);return position;
}
std::uint8_t copy(const char* name,int priority){
    std::uint16_t checksum=64490;std::uint8_t success=0;char source[260];std::strcpy(source,settings.source);std::strcat(source,name);Dword file;
    if(!open_sync(source,65,priority,&file))return 0;
    Dword bytes=size_sync(file,priority);std::uint64_t available=0;if(settings.free_space_limit>=0)available=backend::free_space(settings.destination);
    std::int64_t signed_bytes=static_cast<std::int32_t>(bytes);std::uint64_t used=database?database->bytes:0;
    std::int64_t remaining=static_cast<std::int64_t>(available-signed_bytes);
    if(static_cast<std::int64_t>(used+signed_bytes)<settings.byte_limit&&remaining>settings.free_space_limit){
        char destination[260],temporary[260];std::strcpy(destination,settings.destination);std::strcat(destination,name);create_directories(destination);
        std::strcpy(temporary,settings.destination);std::strcat(temporary,"C_TMP00_.DAT");HANDLE output=INVALID_HANDLE_VALUE;
        if(database&&database->partial_name[0]&&!std::strcmp(database->partial_name,name)){
            output=CreateFileA(temporary,GENERIC_READ,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_TEMPORARY,nullptr);
            if(output!=INVALID_HANDLE_VALUE){DWORD actual=GetFileSize(output,nullptr);
                if(actual==database->partial_bytes){std::uint8_t buffer[8192];DWORD got;ReadFile(output,buffer,8192,&got,nullptr);
                    while(got==8192){checksum=static_cast<std::uint16_t>(kernel::checksum16(buffer,8192,checksum));ReadFile(output,buffer,8192,&got,nullptr);}
                    CloseHandle(output);output=INVALID_HANDLE_VALUE;
                    if(database->partial_checksum==checksum){output=CreateFileA(temporary,GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_TEMPORARY,nullptr);
                        SetFilePointer(output,database->partial_bytes,nullptr,FILE_BEGIN);}else checksum=64490;
                }else{CloseHandle(output);output=INVALID_HANDLE_VALUE;}}
            database->partial_checksum=0;database->partial_name[0]=0;database->partial_bytes=0;
        }
        if(output==INVALID_HANDLE_VALUE){output=CreateFileA(temporary,GENERIC_WRITE,0,nullptr,TRUNCATE_EXISTING,FILE_ATTRIBUTE_TEMPORARY,nullptr);
            if(output==INVALID_HANDLE_VALUE){kernel::print::write(7,"FCACHE_cachefilestream - first open attempt failed on file %s.\n",name);
                output=CreateFileA(temporary,GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY,nullptr);}}
        if(output!=INVALID_HANDLE_VALUE){Entry* entry=nullptr;if(database)entry=static_cast<Entry*>(kernel::binary_search(name,database+1,database->count,261,compare_entry));if(!entry)entry=insert(name);
            Dword copied=stream(file,output,priority,&checksum);CloseHandle(output);
            if(!stopping&&copied!=~Dword(0)){kernel::lock_enter(&lock);std::uint64_t total=database->bytes+static_cast<std::int32_t>(copied);
                if(static_cast<std::int64_t>(total)<settings.byte_limit&&remaining>settings.free_space_limit){database->bytes=total;MoveFileA(temporary,destination);entry->cached=1;success=1;kernel::lock_leave(&lock);}
                else{if(!DeleteFileA(temporary))report_error(0);kernel::lock_leave(&lock);}}
            else{database->partial_bytes=copied;std::strcpy(database->partial_name,name);database->partial_checksum=checksum;dirty=1;}
        }
    }
    close_sync(file,priority);return success;
}
}
