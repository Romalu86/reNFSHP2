#include "nfs/graphics/resources/eagl_objects.hpp"
#include <initializer_list>
namespace nfs::graphics::viewports {
Dword identity_values[16]={1065353216,0,0,0,0,1065353216,0,0,0,0,1065353216,0,0,0,0,1065353216};  
Dword* identity=identity_values;  

void set_owner(Dword* object,Dword* owner){object[0]=reinterpret_cast<Dword>(owner);}
// Receives the embedded state at byte offset 12.
Dword* state_construct(Dword* object,Dword* owner){object[6]=object[7]=object[9]=0;reinterpret_cast<std::uint8_t*>(object)[348]=0;
    object[352/4]=0;object[344/4]=1065353216;object[356/4]=reinterpret_cast<Dword>(owner);return object;}

Dword* construct(Dword* object,Dword* render_context){set_owner(object,object);state_construct(object+3,object);object[1]=object[2]=0;
    for(int start:{52/4,116/4,180/4})for(int i=0;i<16;++i)object[start+i]=identity[i];object[10]=0;object[9]=reinterpret_cast<Dword>(render_context);return object;}

Dword* construct_target(Dword* object,Dword* target){set_owner(object,object);state_construct(object+3,object);object[1]=object[2]=0;
    for(int start:{52/4,116/4,180/4})for(int i=0;i<16;++i)object[start+i]=identity[i];object[9]=0;object[10]=reinterpret_cast<Dword>(target);return object;}

void destroy(Dword*){}

void base_destroy(Dword*){}

void set_data(Dword* object,Dword value){object[2]=value;}

Dword data(const Dword* object){return object[2];}

void parameters(const Dword* object,Dword* first,Dword* second,Dword* third,Dword* fourth){*first=object[268/4];*second=object[272/4];*third=object[276/4];*fourth=object[280/4];}

void set_color(Dword* object,Dword color){object[12]=color;}

Dword* matrix(Dword* object){return object+116/4;}

Dword* current(Dword* render_context){return reinterpret_cast<Dword*>(render_context[208/4]);}
// The caller supplies the render_context-owner indirection.
void select(Dword** owner,Dword* viewport){(*owner)[208/4]=reinterpret_cast<Dword>(viewport);}

Dword register_symbol(const char* name,Dword value){eagl::pool_register(&eagl::external_symbols,name,value);return 0;}
}
namespace nfs::graphics {
// Allocation failure still reaches the member store.
Dword* create_viewport(Dword* render_context){auto* object=static_cast<Dword*>(eagl::allocator(372,"EAGL::Viewport new"));if(object)viewports::construct(object,render_context);
    *reinterpret_cast<volatile Dword*>(reinterpret_cast<std::uintptr_t>(object)+364)=render_context[212/4];render_context[212/4]=reinterpret_cast<Dword>(object);return object;}
// A missing list member retains the invalid dereference path.
void destroy_viewport(Dword* render_context,Dword* viewport){auto* removed=reinterpret_cast<Dword*>(render_context[212/4]);
    if(removed==viewport)render_context[212/4]=removed[364/4];else{Dword* previous=removed;
        while(previous&&previous[364/4]!=reinterpret_cast<Dword>(viewport))previous=reinterpret_cast<Dword*>(previous[364/4]);
        removed=reinterpret_cast<Dword*>(previous[364/4]);previous[364/4]=removed[364/4];}
    viewports::destroy(removed);eagl::deallocator(removed,372);if(render_context[208/4]==reinterpret_cast<Dword>(viewport))render_context[208/4]=0;}
}
