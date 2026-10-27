#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::tar {
// Authorized driver compatibility: the constructor leaves word5 unwritten, but bind forwards it
// to D3DTSS_MAXANISOTROPY. Initialize the isotropic level; explicit SetAnisotropy still overrides
// it.
Dword* base_construct(Dword* object){object[1]=object[2]=object[3]=object[4]=1;object[0]=0;object[5]=1;object[6]=2;object[7]=0;object[8]=2;object[9]=0;return object;}

Dword* shared_construct(Dword* object){Dword flags=(object[12]&~2u)|1;for(int i=0;i<8;++i)object[i]=0;object[8]=~0u;object[9]=object[10]=object[11]=0;object[12]=flags;object[13]=0;return object;}

void shared_destroy(Dword* object){if(object[6]){backend::com(reinterpret_cast<Dword*>(object[6]),8);object[6]=0;}if(object[7]){backend::com(reinterpret_cast<Dword*>(object[7]),8);object[7]=0;}if(object[13]){eagl::deallocator(reinterpret_cast<void*>(object[13]),object[3]*4);object[13]=0;}}

Dword* extension_construct(Dword* extension,Dword* object){*extension=reinterpret_cast<Dword>(object);return extension;}

void extension_destroy(Dword* extension){auto*& head=backend::unresolved::texture_links;for(Dword* node=head;node;){auto* next=reinterpret_cast<Dword*>(node[3]);auto* previous=reinterpret_cast<Dword*>(node[2]);if(node[0]==*extension){if(head==node){head=next;if(next)next[2]=0;}else{if(next)next[2]=reinterpret_cast<Dword>(previous);if(previous)previous[3]=reinterpret_cast<Dword>(next);}node[0]=node[1]=node[2]=node[3]=0;eagl::deallocator(node,16);}node=next;}}

Dword* construct(Dword* object){base_construct(object);extension_construct(object+10,object);auto* shared=static_cast<Dword*>(eagl::allocator(56,"EAGLInternal::SharedData new"));if(shared)shared_construct(shared);object[9]=reinterpret_cast<Dword>(shared);shared[12]|=2;++shared[0];shared[9]=0;for(int i=1;i<=5;++i)shared[i]=0;shared[8]=~0u;shared[10]=shared[11]=0;return object;}

Dword* copy_construct(Dword* object,Dword* source){base_construct(object);extension_construct(object+10,object);for(int i=0;i<10;++i)object[i]=source[i];++reinterpret_cast<Dword*>(object[9])[0];return object;}
// Call both destructors on the last allocated shared object.
void destroy(Dword* object){auto* shared=reinterpret_cast<Dword*>(object[9]);--shared[0];shared=reinterpret_cast<Dword*>(object[9]);if(!shared[0]){if(backend::unresolved::texture_values[object[0]]==shared[6]){if(backend_device)backend::com(backend_device,244,object[0],Dword(0));backend::unresolved::texture_values[object[0]]=0;}shared_destroy(reinterpret_cast<Dword*>(object[9]));}shared=reinterpret_cast<Dword*>(object[9]);if(!shared[0]&&(shared[12]&2)){if(shared){shared_destroy(shared);eagl::deallocator(shared,56);}object[9]=0;}extension_destroy(object+10);}

Dword* shape(Dword* object){return reinterpret_cast<Dword*>(reinterpret_cast<Dword*>(object[9])[9]);}

void destructor(Dword* object){destroy(object);}

void runtime_release(Dword* object,int count){for(int i=0;i<count;++i)destroy(object+i*12);eagl::deallocator(object,static_cast<Dword>(count)*48);}

std::uint8_t set_stage(Dword* extension,Dword stage){reinterpret_cast<Dword*>(*extension)[0]=stage;return 1;}

std::uint8_t replace_shared(Dword* extension,Dword* incoming){
    auto object=[&]{return reinterpret_cast<Dword*>(*extension);};
    auto* shared=reinterpret_cast<Dword*>(object()[9]);
    --shared[0];
    Dword* current=object();
    shared=reinterpret_cast<Dword*>(current[9]);
    if(!shared[0]){
        if(backend::unresolved::texture_values[current[0]]==shared[6]){
            int result=backend::com(backend_device,244,current[0],Dword(0));
            backend::unresolved::texture_values[object()[0]]=0;
            if(result<0)return 0;
        }
        shared_destroy(reinterpret_cast<Dword*>(object()[9]));
    }
    shared=reinterpret_cast<Dword*>(object()[9]);
    if(!shared[0]&&(shared[12]&2)&&shared){shared_destroy(shared);eagl::deallocator(shared,56);}
    ++reinterpret_cast<Dword*>(incoming[9])[0];
    Dword incoming_shared=incoming[9];
    object()[9]=incoming_shared;
    return 1;
}
}
