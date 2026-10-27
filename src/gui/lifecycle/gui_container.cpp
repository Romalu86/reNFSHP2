#include "nfs/gui/lifecycle/gui.hpp"
#include <cstring>
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::gui {
// Deleting destructor adapters retain ECX=this and RET 4; EDX is unused.

static void* __fastcall base_delete(Object* object,void*,unsigned flags){object->methods=base_methods;if(flags&1)release(object);return object;}
const Destroy base_methods[]={reinterpret_cast<Destroy>(base_delete)};
static void* __fastcall element_delete(Element*,void*,unsigned);
extern const Destroy element_methods[]={reinterpret_cast<Destroy>(element_delete)};

static void* __fastcall element_delete(Element* e,void*,unsigned flags){e->methods=element_methods;if(e->dispose&&e->value)e->dispose(e->value);e->value=nullptr;e->dispose=nullptr;e->methods=base_methods;if(flags&1)release(e);return e;}
static void* __fastcall container_delete(Container* c,void*,unsigned flags){container_destruct(c);if(flags&1)release(c);return c;}
const Destroy container_methods[]={reinterpret_cast<Destroy>(container_delete)};

void dispose_object(void* object){if(object)destroy(static_cast<Object*>(object));}
void destroy(Object* object){(*object->methods)(object,1);}

void container_construct(Container* c){c->methods=container_methods;c->elements=nullptr;c->count=0;c->flags=0;}

void container_destruct(Container *container) {
    container->methods = container_methods;
    try {
        container_clear(container);
    } catch (...) {
        container->methods = base_methods;
        throw;
    }
    container->methods = base_methods;
}

Dword container_clear(Container* c){Dword count=c->count;if(count){auto** p=c->elements;do{if(*p){destroy(reinterpret_cast<Object*>(*p));*p=nullptr;}++p;}while(--count);}
    c->count=0;if(c->elements){kernel::memory_free(c->elements);c->elements=nullptr;}return 0;}

void* reallocate(void* memory,const char*,int bytes,Dword flags){if(!memory)return kernel::memory_allocate(nullptr,bytes,pool|flags);
    void* resized=kernel::memory_resize(memory,bytes);int extent=kernel::memory_size(resized);
    if(extent!=bytes){void* replacement=kernel::memory_allocate(nullptr,bytes,pool|flags);
        if(replacement){if(extent<=bytes)bytes=extent;if(bytes>0)kernel::memory_copy(replacement,resized,bytes);}kernel::memory_free(resized);resized=replacement;}return resized;}

Dword container_append(Container *container, void *value, Dispose dispose) {
    const int bytes = container->count * 4 + 4;
    void *storage = container->elements;
    if (!storage)
        storage = kernel::memory_allocate(nullptr, bytes, pool | container->flags);
    else {
        void *resized = kernel::memory_resize(storage, bytes);
        const int extent = kernel::memory_size(resized);
        if (extent != bytes) {
            storage = kernel::memory_allocate(nullptr, bytes, pool | container->flags);
            if (storage) {
                const int copied = bytes < extent ? bytes : extent;
                if (copied > 0)
                    kernel::memory_copy(storage, resized, copied);
            }
            kernel::memory_free(resized);
        } else
            storage = resized;
    }
    auto **entries = static_cast<Element **>(storage);
    container->elements = entries;
    if (entries) {
        auto *element = static_cast<Element *>(
            allocate_flags(sizeof(Element), container->flags, "GCElement"));
        if (element) {
            element->value = value;
            const Dword index = container->count;
            element->methods = element_methods;
            element->dispose = dispose;
            entries[index] = element;
            ++container->count;
            return 0;
        }
    }
    return 1;
}

Dword container_insert(Container* c,void* value,Dword index,Dispose dispose){Dword count=c->count;int bytes=(count+1)*4;
    if(index<count){auto** elements=static_cast<Element**>(kernel::memory_allocate(nullptr,bytes,pool|c->flags));if(!elements)return 1;
        auto* e=static_cast<Element*>(allocate_flags(12,c->flags,"GCElement"));if(e){e->value=value;e->dispose=dispose;e->methods=element_methods;
            if(static_cast<int>(index*4)>0)kernel::memory_copy(elements,c->elements,index*4);elements[index]=e;
            int tail=(count-index)*4;if(tail>0)kernel::memory_copy(elements+index+1,c->elements+index,tail);
            kernel::memory_free(c->elements);c->elements=elements;c->count=count+1;return 0;}
    }else{auto** entries=static_cast<Element**>(reallocate(c->elements,"GContainer",bytes,c->flags));c->elements=entries;if(entries){
        auto* e=static_cast<Element*>(allocate_flags(12,c->flags,"GCElement"));if(e){e->methods=element_methods;e->value=value;e->dispose=dispose;
            entries[count]=e;++c->count;return 0;}}}return 1;}

void* container_find(Container* c,Compare compare,const void* key){if(!compare||!c->count)return nullptr;Dword count=c->count,lower=0,upper=count,previous=~Dword(0);
    auto value=[&](Dword i)->void*{return c->elements[i]?c->elements[i]->value:nullptr;};
    void* item=value(0);if(item&&compare(item,key)>0)return nullptr;item=value(count-1);if(item&&compare(item,key)<0)return nullptr;
    for(;;){Dword middle=static_cast<std::int32_t>(lower+upper)/2;if(middle==previous||middle>=count)return nullptr;
        item=value(middle);if(!item)return nullptr;int order=compare(item,key);if(!order)return item;previous=middle;if(order>0)upper=middle;else lower=middle;}}

Dword container_sorted_insert(Container* c,Compare compare,const void* key,void* item,Dispose dispose){if(!compare)return 2;Dword count=c->count;if(!count)return container_append(c,item,dispose);
    auto value=[&](Dword i)->void*{return c->elements[i]?c->elements[i]->value:nullptr;};
    void* current=value(0);if(current&&compare(current,key)>0)return container_insert(c,item,0,dispose);
    current=value(count-1);if(current&&compare(current,key)<0)return container_append(c,item,dispose);
    Dword lower=0,upper=count,previous=~Dword(0),middle;int order=0;
    for(;;){middle=static_cast<std::int32_t>(lower+upper)/2;if(middle==previous||middle>=count)break;
        current=value(middle);if(!current)break;order=compare(current,key);if(!order)return container_insert(c,item,middle,dispose);
        previous=middle;if(order>0)upper=middle;else lower=middle;}
    return container_insert(c,item,order<0?middle+1:middle,dispose);}
// Allocation failure follows deletion of the element.
Dword container_remove(Container* c,void* item){if(!item)return 2;Dword count=c->count,remaining=count,index=0;
    for(Dword i=0;i<count;++i)if(c->elements[i]&&c->elements[i]->value==item){destroy(reinterpret_cast<Object*>(c->elements[i]));c->elements[i]=nullptr;remaining=count-1;index=i;break;}
    if(remaining==count)return 6;if(remaining){auto** elements=static_cast<Element**>(kernel::memory_allocate(nullptr,remaining*4,pool|c->flags));
        if(!elements)return 1;if(static_cast<int>(index*4)>0)kernel::memory_copy(elements,c->elements,index*4);
        int tail=(count-index)*4-4;if(tail>0)kernel::memory_copy(elements+index,c->elements+index+1,tail);
        kernel::memory_free(c->elements);c->elements=elements;c->count=remaining;return 0;}
    kernel::memory_free(c->elements);c->elements=nullptr;c->count=0;return 0;}
}
