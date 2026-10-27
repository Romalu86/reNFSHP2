#include "nfs/graphics/backend/eagl.hpp"
#include <cstring>
namespace nfs::eagl {
#include "../loader/eagl_messages.inc"
template<class T>static T* pointer(Dword p){return reinterpret_cast<T*>(p);}
// RenderMethod constructors precede every other type;

void construct_objects(Loader* loader){int count=symbol_count(loader),needed=0;
    for(int i=0;i<count;++i){SymbolView view;symbol_at(loader,&view,i);if(view.defined){std::uint8_t found;
        if(pool_lookup(&constructors.construct,view.type,&found)&&found)++needed;}}
    if(needed>0){loader->destructors=static_cast<Destruction*>(allocator(needed*8,"EAGL::dynamic destructor list"));int written=0;
        for(int pass=0;pass<2;++pass)for(int i=0;i<count;++i){SymbolView view;symbol_at(loader,&view,i);
            if(view.defined){bool render_method=std::strncmp("RenderMethod",view.type,13)==0;if((pass==0)==render_method){std::uint8_t found;
                Dword constructor=pool_lookup(&constructors.construct,view.type,&found);if(!found)constructor=0;if(constructor){
                    Dword destructor=pool_lookup(&constructors.destroy,view.type,&found);if(!found)destructor=0;
                    reinterpret_cast<void(__cdecl*)(Dword,Loader*)>(constructor)(view.address,loader);
                    Destruction *volatile &entries = loader->destructors;
                    entries[written].callback = reinterpret_cast<Destructor>(destructor);
                    entries[written].value = view.address;
                    ++written;}}}}
        loader->destructor_count=written;return;}loader->destructor_count=needed;
}

void resolve(Loader* loader){Record* record=loader->record;if(!record||record->resolved)return;record->resolved=1;
    Section* sections=record->sections;ElfHeader* header=record->header;
    for(int i=0;i<header->shnum;++i){Section& section=sections[i];if(section.type!=9)continue;
        auto* relocations=pointer<Relocation>(section.offset);auto* symbols=pointer<Symbol>(section.link);Dword destination=section.info;int count=static_cast<int>(section.size)/8;
        for(int j=0;j<count;++j){Relocation& relocation=relocations[j];Symbol& symbol=symbols[relocation.info>>8];Dword value=0;bool complete=false;
            while((symbol.info&15)<4){if(symbol.section&&symbol.section<header->shnum){value=sections[symbol.section].offset+symbol.value;complete=true;break;}
                symbol.binding=2;Dword resolved=0;for(Record* other=records;other;other=other->next)if(other!=record){resolved=record_lookup(other,record->strings+symbol.name);if(resolved)break;}
                if(resolved){symbol.section=1;symbol.value=resolved-sections[1].offset;continue;}
                std::uint8_t found=0;if(record->resolve)resolved=record->resolve(record->strings+symbol.name,&found);
                if(!record->resolve||!found)resolved=pool_lookup(&external_symbols,record->strings+symbol.name,&found);
                if(found){symbol.section=1;symbol.value=resolved-sections[1].offset;continue;}
                const char* name=record->strings+symbol.name;const char* type=name+std::strlen(name)+1;if(*type==127)++type;else --type;
                if(!std::strncmp("RUNTIME_ALLOC::",name,15)){Dword constructor=pool_lookup(&runtime_allocators.construct,type,&found);if(!found)constructor=0;
                    if(constructor){Dword destructor=pool_lookup(&runtime_allocators.destroy,type,&found);if(!found)destructor=0;
                        Dword context;std::uint8_t owned=0;
                        using RuntimeConstruct=Dword(__cdecl*)(const char*,Loader*,Dword*,std::uint8_t*);
                        resolved=reinterpret_cast<RuntimeConstruct>(constructor)(name+15,loader,&context,&owned);
                        if(owned&&resolved){auto* entry=static_cast<RuntimeDestruction*>(allocator(16,"EAGL::DynamicLoader::RuntimeAllocDestructorEntry new"));
                            if(entry){entry->callback=reinterpret_cast<RuntimeDestroy>(destructor);entry->value=resolved;entry->context=context;entry->next=nullptr;}
                            entry->next=loader->runtime_destructors;loader->runtime_destructors=entry;}
                        symbol.section=1;symbol.value=resolved-sections[1].offset;continue;}}
                report(0,loader_unresolved_symbol_message,name);value+=symbol.value;complete=true;break;}
            if(!complete)report(0,loader_unknown_symbol_type_message,record->strings+symbol.name,symbol.info&15);
            auto* target=pointer<Dword>(relocation.offset+destination);
            switch(relocation.info&255){case 0:case 1:case 3:break;case 2:*target+=value;break;
            case 4:value+=(*target&67108863)*4;if(((reinterpret_cast<Dword>(target+1)^value)&4227858432u)!=0)report(0,loader_jump_out_of_range_message,target+1,value);
                *target=((value>>2)^*target)&67108863^*target;break;
            case 5:value>>=16;[[fallthrough]];case 6:*reinterpret_cast<std::uint16_t*>(target)+=static_cast<std::uint16_t>(value);break;
            case 7:case 12:report(0,loader_gp_relative_relocation_message);break;case 8:report(0,loader_mips_literal_relocation_message);break;
            case 9:report(0,loader_mips_got_relocation_message);break;default:report(0,loader_unsupported_relocation_message,relocation.info&255);break;}}
    }construct_objects(loader);
}
}
