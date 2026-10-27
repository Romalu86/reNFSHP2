#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include <cstring>
#include <cstdio>
namespace nfs::ini {
using namespace gui;
CachedDocument* cached=nullptr;Dword load_pool=0;static const String &empty = lifecycle::construction::unresolved::shared_string;
static const String* name_string(PoolString* name){return name?&name->string:&empty;}

static void* __fastcall cached_delete(CachedDocument* doc,void*,unsigned flags){clear(&doc->document);if(flags&1)gui::release(doc);return doc;}
static const Destroy cached_methods[]={reinterpret_cast<Destroy>(cached_delete)};

static CachedDocument *cached_construct(CachedDocument *doc, const char *name) {
    construct(&doc->document, nullptr, name);
    try {
        doc->next = nullptr;
        doc->previous = nullptr;
        doc->references = 0;
        doc->document.methods = cached_methods;
        void *text = files::file::try_load(name, load_pool);
        if (text) {
            parse(&doc->document, static_cast<const char *>(text), kernel::memory_size(text), 0,
                  1);
            kernel::memory_free(text);
        }
    } catch (...) {
        // Unwind the completed Document base.
        clear(&doc->document);
        throw;
    }
    return doc;
}

CachedDocument* acquire(const char* name,std::uint8_t retain){CachedDocument* candidate=nullptr;int count=0;
    for(auto* current=cached;current;current=current->next){if(string_equal_folded(name_string(current->document.name),name)){current->references = int(Dword(current->references) + 1);return current;}
        if(current->references<1)candidate=current;++count;}
    auto *result = static_cast<CachedDocument *>(allocate(44));
    if (result) {
        try {
            cached_construct(result, name);
        } catch (...) {
            // Clear the Document before freeing the allocation. cached_construct has already unwound the
            // Document.
            gui::release(result);
            throw;
        }
    }
    if(retain){if(count>999){if(!candidate)return result;auto* previous=candidate->previous;if(previous)previous->next=candidate->next;
            if(candidate->next)candidate->next->previous=previous;destroy(reinterpret_cast<Object*>(candidate));}
        if(cached)cached->previous=result;result->references = int(Dword(result->references) + 1);result->next=cached;cached=result;}
    return result;}

void release(CachedDocument* doc){for(auto* current=cached;current;current=current->next)if(current==doc){current->references = int(Dword(current->references) - 1);return;}
    if(doc)destroy(reinterpret_cast<Object*>(doc));}

void flush_cache(){auto* current=cached;while(current){auto* next=current->next;destroy(reinterpret_cast<Object*>(current));current=next;}cached=nullptr;}

static void* __fastcall reader_delete(Reader* reader,void*,unsigned flags){reader_clear(reader);if(flags&1)gui::application_release(reader);return reader;}
static const Destroy reader_methods[]={reinterpret_cast<Destroy>(reader_delete)};
Reader *reader_construct(Reader *reader, const char *name, std::uint8_t retain) {
    reader->modified = 0;
    reader->document = nullptr;
    reader->methods = reader_methods;
    reader->document = acquire(name, retain);
    return reader;
}
void reader_clear(Reader* reader){reader->methods=reader_methods;if(reader->modified&&reader->document)save(&reader->document->document);release(reader->document);}

Section* reader_section(Reader* reader,const char* section_name){if(reader->document){Container& sections=reader->document->document.sections;
    for(int i=0;i<static_cast<int>(sections.count);++i){Element* e=sections.elements[i];auto* section=e?static_cast<Section*>(e->value):nullptr;
        if(section&&string_equal_folded(name_string(section->name),section_name))return section;}}return nullptr;}

Key* reader_key(Reader* reader,const char* section_name,const char* key_name){Section* section=reader_section(reader,section_name);if(section){
    for(int i=0;i<static_cast<int>(section->keys.count);++i){Element* e=section->keys.elements[i];auto* key=e?static_cast<Key*>(e->value):nullptr;
        if(key&&string_equal_folded(name_string(key->name),key_name))return key;}}return nullptr;}
static bool nonempty(Key* key){if(!key||key->value.length<=0)return false;const void* data=string_data(&key->value);return data&&(key->value.wide?*static_cast<const std::uint16_t*>(data)!=0:*static_cast<const char*>(data)!=0);}

std::uint8_t get_string(Reader* r,const char* section,const char* key,char* out,Dword capacity){if(!r->document||!out||!capacity)return 0;
    Key* value=reader_key(r,section,key);if(!nonempty(value))return 0;std::strncpy(out,string_ascii(&value->value),capacity);out[capacity-1]=0;return 1;}

bool get_int(Reader* r,const char* section,const char* key,int* out){Key* value=reader_key(r,section,key);return nonempty(value)&&std::sscanf(string_ascii(&value->value),"%d",out)!=0;}
bool get_float(Reader* r,const char* section,const char* key,float* out){Key* value=reader_key(r,section,key);return nonempty(value)&&std::sscanf(string_ascii(&value->value),"%f",out)!=0;}
// (BOOL is a raw parsed DWORD, not !!value).
std::uint8_t get_bool(Reader* r,const char* section,const char* key,Dword* out){if(!r->document)return 0;Dword value=*out;Key* found=reader_key(r,section,key);
    if(nonempty(found)&&std::sscanf(string_ascii(&found->value),"%d",&value)){*out=value;return 1;}return 0;}
std::uint8_t get_short(Reader* r,const char* section,const char* key,short* out){if(!r->document)return 0;int value=*out;Key* found=reader_key(r,section,key);
    if(nonempty(found)&&std::sscanf(string_ascii(&found->value),"%d",&value)){*out=static_cast<short>(value);return 1;}return 0;}
}
