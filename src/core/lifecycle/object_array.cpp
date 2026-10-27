#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <exception>
namespace nfs::lifecycle::construction::objects {
void destroy_unwound_elements(void *,int,int,void(*)(void*));
}
namespace nfs::lifecycle::construction {
using ArrayMethod=void(*)(void*);
template<class Construct>
static void construct_elements(void* array,int stride,int count,Construct construct,ArrayMethod destroy){
    int completed=0;
    auto next=reinterpret_cast<std::uintptr_t>(array);
    const auto step=static_cast<std::uintptr_t>(stride);
    try{
        for(;completed<count;++completed){
            construct(reinterpret_cast<void*>(next));
            next+=step;
        }
    }catch(...){
        objects::destroy_unwound_elements(reinterpret_cast<void*>(next),stride,completed,destroy);
        throw;
    }
}

void construct_array(void* array,int stride,int count,ArrayMethod construct,ArrayMethod destroy){
    construct_elements(array,stride,count,construct,destroy);
}
// The iterator discards EAX; preserve each constructor's C++ return type.
void construct_array(void* array,int stride,int count,void*(*construct)(void*),ArrayMethod destroy){
    construct_elements(array,stride,count,construct,destroy);
}
}
namespace nfs::lifecycle::construction::objects {
// The filter terminates on a C++ exception during unwind.
void destroy_unwound_elements(void* end,int stride,int count,void(*destroy)(void*)){
    auto next=reinterpret_cast<std::uintptr_t>(end);
    const auto step=static_cast<std::uintptr_t>(stride);
    try{
        for(;;){
            count=static_cast<int>(Dword(count)-1u);
            if(count<0)break;
            next-=step;
            destroy(reinterpret_cast<void*>(next));
        }
    }catch(...){
        std::terminate();
    }
}

void destroy_array(void* array,int stride,int count,void(*destroy)(void*)){
    const auto step=static_cast<std::uintptr_t>(stride);
    auto next=reinterpret_cast<std::uintptr_t>(array)+step*Dword(count);
    try{
        for(;;){
            count=static_cast<int>(Dword(count)-1u);
            if(count<0)break;
            next-=step;
            destroy(reinterpret_cast<void*>(next));
        }
    }catch(...){
        destroy_unwound_elements(reinterpret_cast<void*>(next),stride,count,destroy);
        throw;
    }
}

void* pointer_construct(void* p){static_cast<Dword*>(p)[0]=0;return p;}

void pointer_destroy(void*){}
}
