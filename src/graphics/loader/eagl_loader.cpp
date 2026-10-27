#include "nfs/graphics/backend/eagl.hpp"
#include <cstring>
namespace nfs::eagl {
#include "eagl_messages.inc"
Record* records=nullptr;
static Dword word(const void* p){return reinterpret_cast<Dword>(p);}
template<class T>static T* pointer(Dword p){return reinterpret_cast<T*>(p);}
static Dword offset(Loader* loader,Dword value){if(value<loader->split)return word(loader->header)+value;
    if(loader->second)return word(loader->second)-loader->split+value;if(value<=loader->split)return word(loader->header)+value;return 0;}

Dword dynamic_hash(const char* name){Dword hash=0;while(*name){hash=hash*16+static_cast<std::int32_t>(*name++);Dword high=hash&4026531840u;if(high)hash^=high>>24;hash&=~high;}return hash&255;}
// Caller owns and supplies writable, original ELF data.
void prepare(Loader* loader,Resolve resolver){Record temporary;temporary.header=loader->header;temporary.next=nullptr;temporary.strings=nullptr;temporary.resolved=0;temporary.count=0;
    temporary.sections=pointer<Section>(offset(loader,loader->header->shoff));temporary.symbols=nullptr;
    auto* sections=temporary.sections;for(Dword i=0;i<loader->header->shnum;++i)sections[i].offset=offset(loader,sections[i].offset);
    auto* section_names=pointer<char>(sections[loader->header->shstrndx].offset);
    for(Dword i=0;i<loader->header->shnum;++i){Section& section=sections[i];Dword type=section.type;
        switch(type){case 0:case 1:break;
        case 2:section.link=sections[section.link].offset;if(!std::strncmp(".symtab",section_names+section.name,8)){
            temporary.symbols=pointer<Symbol>(section.offset);temporary.count=static_cast<int>(section.size)/16;}break;
        case 3:if(!std::strncmp(".strtab",section_names+section.name,8)){temporary.strings=pointer<char>(section.offset);char* current=temporary.strings;
            for(int remaining=section.size;remaining>0;){Dword length=static_cast<Dword>(std::strlen(current));
                if(current[0]=='_'&&current[1]=='_'){char* type_name=current+2;char* separator=std::strstr(type_name,":::");if(separator){*separator=0;
                    char saved_type[128];std::strcpy(saved_type,type_name);char* name=separator+3;Dword bytes=static_cast<Dword>(std::strlen(name))+1;
                    std::memmove(current,name,bytes);current[bytes]=127;std::strcpy(current+bytes+1,saved_type);}}
                remaining-=length+1;current+=length+1;}}break;
        case 4:if(section.size)report(0,loader_rela_section_message);break;
        case 5:case 6:report(0,loader_hash_section_message);break;
        case 8:if(section.size)report(0,loader_nobits_data_message);break;
        case 9:section.info=sections[section.info].offset;section.link=sections[section.link].offset;break;
        case 10:report(0,loader_shlib_section_message);break;
        case 11:report(0,loader_dynsym_section_message);break;
        case 1879048198:case 2147480608:case 2147480609:break;
        default:report(0,loader_unknown_section_message,type,i);break;}}
    if(!temporary.symbols)report(0,loader_missing_symbol_table_message);
    for(int& bucket:temporary.buckets)bucket=-1;
    temporary.links=static_cast<int*>(allocator(temporary.count*4,"EAGL::dynamic symbols"));
    auto* record=static_cast<Record*>(allocator(1064,"EAGL::HashPointer new"));
    if(record){record->next=nullptr;record->strings=nullptr;record->resolved=0;record->count=0;record->symbols=nullptr;record->sections=nullptr;record->header=nullptr;record->links=nullptr;record->resolve=nullptr;}
    for(int i=0;i<temporary.count;++i){Dword hash=dynamic_hash(temporary.strings+temporary.symbols[i].name);temporary.links[i]=temporary.buckets[hash];temporary.buckets[hash]=i;}
    temporary.resolve=resolver;std::memcpy(record,&temporary,1064);record->next=records;if(records)records->previous=record;record->previous=nullptr;records=record;loader->record=record;
}

int symbol_count(Loader* loader){return loader->record?loader->record->count:0;}
// In invalid-index results, only name/address are defined.
void symbol_at(Loader* loader,SymbolView* view,int index){Record* record=loader->record;if(!record||index<0||index>=record->count){view->name=nullptr;view->address=0;return;}
    Symbol& symbol=record->symbols[index];const char* name=record->strings+symbol.name;Dword length=static_cast<Dword>(std::strlen(name));
    const char* type=name+length+1;if(*type==127)++type;else --type;
    view->name=name;view->type=type;view->address=0;view->defined=symbol.binding!=2;
    if(symbol.binding==1)view->address=symbol.value;else if(symbol.section&&symbol.section<record->header->shnum)view->address=record->sections[symbol.section].offset+symbol.value;
}

Dword record_lookup(Record* record,const char* name){Dword index=record->buckets[dynamic_hash(name)];while(index!=4294967295u){
    if(static_cast<Dword>(record->count)<index)report(0,loader_invalid_symbol_chain_message,index,record->count);
    Symbol& symbol=record->symbols[index];if(!std::strcmp(name,record->strings+symbol.name)&&symbol.section&&symbol.section<record->header->shnum)
        return record->sections[symbol.section].offset+symbol.value;index=record->links[index];}return 0;}
// only AL is the success result.
std::uint8_t address(Loader* loader,const char* type,const char* name,Dword* out){*out=0;Record* record=loader->record;if(!record)return 0;
    Dword index=record->buckets[dynamic_hash(name)];while(index!=4294967295u){if(static_cast<Dword>(record->count)<index)report(0,loader_invalid_address_chain_message,index,record->count);
        else{Symbol& symbol=record->symbols[index];const char* symbol_name=record->strings+symbol.name;const char* symbol_type=symbol_name+std::strlen(name)+1;
            if(*symbol_type==127)++symbol_type;else --symbol_type;
            if(!std::strcmp(name,symbol_name)&&!std::strcmp(type,symbol_type)){if(symbol.binding==1){*out=symbol.value;return 1;}
                if(symbol.section&&symbol.section<record->header->shnum){*out=record->sections[symbol.section].offset+symbol.value;return 1;}}}index=record->links[index];}return 0;}

std::uint8_t next_symbol(Loader* loader,const char* type,int* index,SymbolView* out){int count=symbol_count(loader);while(*index<count){SymbolView view;symbol_at(loader,&view,*index);
    if(!std::strcmp(type,view.type)){std::memcpy(out,&view,16);++*index;return 1;}++*index;count=symbol_count(loader);}return 0;}
Dword next_address(Loader* loader,const char* type,int* index,Dword* out){SymbolView view;view.address=0;if(next_symbol(loader,type,index,&view)){*out=view.address;return 1;}return 0;}

void detach(Loader* loader){Record* record=loader->record;if(record){if(!record->previous)records=record->next;else record->previous->next=record->next;
    if(record->next)record->next->previous=record->previous;if(record->links)deallocator(record->links,record->count*4);deallocator(record,1064);loader->record=nullptr;}}

void destroy_objects(Loader* loader){if(loader->destructors){int index=loader->destructor_count;while(--index>=0)loader->destructors[index].callback(loader->destructors[index].value);
    deallocator(loader->destructors,loader->destructor_count*8);loader->destructors=nullptr;}
    auto* current=loader->runtime_destructors;while(current){auto* next=current->next;current->callback(current->value,current->context);deallocator(current,16);current=next;}loader->runtime_destructors=nullptr;}
// The native completion entry returns one without inspecting the loader.
std::uint8_t finish_loading(Loader *) {
    return 1;
}

Loader *loader_construct(Loader *loader, ElfHeader *header, Dword split, Resolve resolver) {
    loader->destructor_count = 0;
    loader->destructors = nullptr;
    loader->runtime_destructors = nullptr;
    loader->second = nullptr;
    loader->header = header;
    loader->split = split;
    prepare(loader, resolver);
    resolve(loader);
    finish_loading(loader);
    return loader;
}

void loader_destroy(Loader* loader){detach(loader);destroy_objects(loader);}
}
