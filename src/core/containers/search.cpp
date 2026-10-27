#include "nfs/core/types/kernel.hpp"
namespace nfs::kernel {
void* binary_search(const void* key,const void* base,Dword count,int width,BinaryCompare compare){
    Dword left=reinterpret_cast<Dword>(base),right=left+(count-1)*width;
    while(left<=right){Dword half=count>>1;if(!half){if(!count)return nullptr;
            return compare(key,reinterpret_cast<void*>(left))==0?reinterpret_cast<void*>(left):nullptr;}
        Dword middle=left+((count&1)?half:half-1)*width;int order=compare(key,reinterpret_cast<void*>(middle));
        if(!order)return reinterpret_cast<void*>(middle);
        if(order<0){right=middle-width;if(!(count&1))--half;}else left=middle+width;count=half;
    }return nullptr;
}
}
