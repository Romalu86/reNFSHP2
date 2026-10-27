#include "nfs/resources/files/filesystem.hpp"
#include "nfs/resources/archives/big_directory.hpp"
#include <cstring>
#include "nfs/core/memory/memory_primitives.hpp"
#include <malloc.h>
namespace nfs::files::backend {
static bool slash(char c){return c=='\\'||c=='/';}
void path_root(const char* path,char* result){
    result[0]=0;std::size_t bytes=std::strlen(path)+1;if(bytes>260)bytes=260;kernel::memory_copy(result,path,bytes);
    char* p=result;while(*p){if(slash(*p)&&slash(p[1]))break;++p;}
    if(*p){p+=2;while(*p&&!slash(*p))++p;if(*p)++p;while(*p&&!slash(*p))++p;p[1]=0;return;}
    p=result;while(*p&&!slash(*p))++p;p[1]=0;
}
bool full_path(const char* path,char* result,Dword capacity){
    char* final_part;DWORD length=GetFullPathNameA(path,capacity,result,&final_part);
    if(length){char root[260];path_root(result,root);DWORD maximum_component,flags;
        if(!GetVolumeInformationA(root,nullptr,0,nullptr,&maximum_component,&flags,nullptr,0))return false;
        if(!(flags&2))CharUpperA(result);if(!(flags&4)){WIN32_FIND_DATAA entry;HANDLE find=FindFirstFileA(path,&entry);
            if(find!=INVALID_HANDLE_VALUE){FindClose(find);std::strcpy(final_part,entry.cFileName);}}
        return true;
    }
    std::size_t bytes=std::strlen(path)+1;if(bytes>260)bytes=260;kernel::memory_copy(result,path,bytes);return false;
}
int search_drive(const char* path,char* result){
    SetLastError(0);DWORD bytes=GetLogicalDriveStringsA(0,nullptr);char* buffer=static_cast<char*>(_alloca(bytes+2047));
    if(!full_path(path,buffer,bytes+2047))files::get_directory(buffer);path_root(buffer,result);GetLogicalDriveStringsA(bytes,buffer);
    DWORD drives=GetLogicalDrives();int i=0;while(*buffer){if(drives&(Dword(1)<<(i&31))){if(!big_name_compare(buffer,result))break;buffer+=std::strlen(buffer)+1;}
        ++i;if(i>31)return 0;}if(i>31)return 0;return i+65;
}
std::uint64_t free_space(const char* root){
    auto* function=GetProcAddress(GetModuleHandleA("kernel32.dll"),"GetDiskFreeSpaceExA");
    if(function){using Query=BOOL(WINAPI*)(LPCSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);ULARGE_INTEGER available,total;
        if(reinterpret_cast<Query>(function)(root,&available,&total,nullptr))return available.QuadPart;
    }else {SetLastError(0);DWORD sectors,sector_bytes,available,total;if(GetDiskFreeSpaceA(root,&sectors,&sector_bytes,&available,&total))return Dword(sectors*sector_bytes*available);}
    return 0;
}
}
