#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
#include <exception>
namespace nfs::ini {
using namespace gui;
static const String &empty = lifecycle::construction::unresolved::shared_string;static const String* name_string(PoolString* name){return name?&name->string:&empty;}

static void* __fastcall key_delete(Key*,void*,unsigned);
static const Destroy key_methods[]={reinterpret_cast<Destroy>(key_delete)};
static void *__fastcall key_delete(Key *key, void *, unsigned flags) {
    key->methods = key_methods;
    try {
        string_release(key->name);
        key->name = nullptr;
        string_destruct(&key->value);
    } catch (...) {
        try {
            string_destruct(&key->value);
            key->methods = base_methods;
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    key->methods = base_methods;
    if (flags & 1)
        gui::release(key);
    return key;
}

static void* __fastcall section_delete(Section*,void*,unsigned);
static const Destroy section_methods[]={reinterpret_cast<Destroy>(section_delete)};
static void *__fastcall section_delete(Section *section, void *, unsigned flags) {
    section->methods = section_methods;
    bool clearing_keys = false;
    try {
        string_release(section->name);
        section->name = nullptr;
        clearing_keys = true;
        container_destruct(&section->keys);
    } catch (...) {
        try {
            if (clearing_keys)
                section->keys.methods = base_methods;
            else
                container_destruct(&section->keys);
            section->methods = base_methods;
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    section->methods = base_methods;
    if (flags & 1)
        gui::release(section);
    return section;
}

static void* __fastcall document_delete(Document* doc,void*,unsigned flags){clear(doc);if(flags&1)gui::release(doc);return doc;}
static const Destroy document_methods[]={reinterpret_cast<Destroy>(document_delete)};

Document *construct(Document *doc, const char *directory, const char *name) {
    container_construct(&doc->sections);
    doc->methods = document_methods;
    try {
        doc->directory = directory ? string_acquire(directory) : nullptr;
        doc->name = name ? string_acquire(name) : nullptr;
        doc->auto_save = 0;
    } catch (...) {
        // Destroy the Container base, then the Object base.
        container_destruct(&doc->sections);
        doc->methods = base_methods;
        throw;
    }
    return doc;
}

void clear(Document *doc) {
    doc->methods = document_methods;
    bool clearing_sections = false;
    try {
        if (doc->auto_save)
            save(doc);
        string_release(doc->directory);
        doc->directory = nullptr;
        string_release(doc->name);
        doc->name = nullptr;
        clearing_sections = true;
        container_destruct(&doc->sections);
    } catch (...) {
        try {
            if (clearing_sections)
                doc->sections.methods = base_methods;
            else
                container_destruct(&doc->sections);
            doc->methods = base_methods;
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    doc->methods = base_methods;
}

Section* find_section(Document* doc,const char* name){Dword count=doc->sections.count;for(Dword i=0;i<count;++i){Element* e=doc->sections.elements[i];auto* section=e?static_cast<Section*>(e->value):nullptr;
    if(section&&!string_compare_inline_ascii(name_string(section->name),name))return section;}return nullptr;}

Section* add_section(Document* doc,const char* name,Dword flags){if(!name)return nullptr;if(flags&1){Dword count=doc->sections.count;
    for(Dword i=0;i<count;++i){Element* e=doc->sections.elements[i];auto* section=e?static_cast<Section*>(e->value):nullptr;
        if(section){const String* text=name_string(section->name);if(string_data(text)&&!string_compare_inline_ascii(text,name))return section;}}}
    auto *section = static_cast<Section *>(allocate_flags(24, 0, "INISection"));
    if (section) {
        container_construct(&section->keys);
        section->methods = section_methods;
        try {
            section->name = string_acquire(name);
        } catch (...) {

            container_destruct(&section->keys);
            section->methods = base_methods;
            gui::release(section);
            throw;
        }
    }
    if(section)container_append(&doc->sections,section,dispose_object);return section;}

void remove_section(Document* doc,const char* name){Dword count=doc->sections.count;for(Dword i=0;i<count;++i){Element* e=doc->sections.elements[i];auto* section=e?static_cast<Section*>(e->value):nullptr;
    if(section&&!string_compare_inline_ascii(name_string(section->name),name)){container_remove(&doc->sections,section);return;}}}

Key* set(Section* section,const char* name,const char* value,Dword flags){if(!name)return nullptr;
    if(flags&1){Dword count=section->keys.count;for(Dword i=0;i<count;++i){Element* e=section->keys.elements[i];auto* key=e?static_cast<Key*>(e->value):nullptr;
        if(key){const String* text=name_string(key->name);if(string_data(text)&&!string_compare_ascii(text,name)){string_assign(&key->value,value);return key;}}}}
    auto *key = static_cast<Key *>(allocate_flags(28, 0, "INIKey"));
    if (key) {
        string_construct(&key->value);
        key->methods = key_methods;
        try {
            key->name = string_acquire(name);
            string_assign(&key->value, value);
        } catch (...) {

            string_destruct(&key->value);
            key->methods = base_methods;
            gui::release(key);
            throw;
        }
    }
    if(key)container_append(&section->keys,key,dispose_object);return key;}

Key* find(Section* section,const char* name){for(Dword i=0;i<section->keys.count;++i){Element* e=section->keys.elements[i];auto* key=e?static_cast<Key*>(e->value):nullptr;
    if(key&&!string_compare_inline_ascii(name_string(key->name),name))return key;}return nullptr;}

void remove(Section* section,const char* name){Key* key=find(section,name);if(key)container_remove(&section->keys,key);}

void rename(Section* section,const char* name){if(section->name)string_release(section->name);section->name=string_acquire(name);}
// appends keys up to the following section header.
const char* parse_section(Section* section,const char* text,const char* end,Dword flags){char value[1024],assignment[1024],line[1024],name[1024];if(!text)return end;
    const char* scan=text;while(*scan!='['&&scan<end){char c=*scan;while(c!='\n'&&c!='\r'){if(scan>=end)goto boundary;c=*++scan;}
        while(scan<end&&*scan<=' ')++scan;if(scan>=end)goto boundary;}
    if(scan)end=scan;
boundary:
    while(text<end){int length=0;scan=text;while(*scan!='\n'&&*scan!='\r'&&scan<end&&length<=1022)line[length++]=*scan++;line[length]=0;
        const char* trimmed=line;while(*trimmed&&*trimmed<=' ')++trimmed;if(*trimmed){std::strncpy(assignment,trimmed,1023);if(split_assignment(assignment,name,value))set(section,name,value,flags);}
        char c=*text;while(c!='\n'&&c!='\r'){if(text>=end)return end;c=*++text;}
        while(text<end&&*text<=' ')++text;if(text>=end)return end;}
    return end;}
// Preserve dereference/check order and signed-byte spaces.
const char* next_line(const char* cursor,const char* end){char c=*cursor;
    while(c!='\n'&&c!='\r'){if(cursor>=end)return nullptr;c=*++cursor;}
    if(cursor<end){while(*cursor<'!'){++cursor;if(cursor>=end)return nullptr;}if(cursor<end)return cursor;}return nullptr;}
// Values retain trailing spaces, quotes and comment text.
Dword split_assignment(const char* line,char* name,char* value){*name=0;*value=0;const char* equal=std::strchr(line,'=');if(!equal)return 0;
    std::size_t count=equal-line;std::strncpy(name,line,count);name[count]=0;
    char* trim=name+count;while(count&&*trim<'!'){*trim=0;--count;--trim;}
    const char* source=equal+1;while(*source&&*source<'!')++source;do{*value++=*source;}while(*source++);return 1;}
// seven 1024-byte stack buffers, section/key merge flags.
void parse(Document* doc,const char* text,int bytes,Dword section_flags,Dword key_flags){
    char key_name[1024],key_value[1024],assignment[1024],section_name[1024],header[1024],line[1024],raw[1024];
    container_clear(&doc->sections);if(!text)return;const char* end=text+bytes;const char* current=text;
    while(current<end){
        if(current){while(current<end){char c=*current;if(c=='[')break;
            while(c!='\n'&&c!='\r'){if(current>=end){current=nullptr;break;}c=*++current;}if(!current)break;
            while(current<end&&*current<=' ')++current;if(current>=end){current=nullptr;break;}}
        }if(!current||current>=end)return;
        int length=0;const char* scan=current;while(*scan!='\n'&&*scan!='\r'&&scan<end&&length<=1022)raw[length++]=*scan++;raw[length]=0;
        const char* trimmed=raw;while(*trimmed&&*trimmed<=' ')++trimmed;
        if(!*trimmed){current=next_line(current,end);if(!current)return;continue;}
        std::strncpy(header,trimmed,1023);const char* body=next_line(current,end);
        const char* opening=std::strchr(header,'[');const char* start=reinterpret_cast<const char*>(reinterpret_cast<Dword>(opening)+1);const char* closing=std::strchr(header,']');current=body;
        if(start&&start<closing){std::strncpy(section_name,start,closing-start);section_name[closing-start]=0;
            Section* section=add_section(doc,section_name,section_flags);if(section){if(!body)return;
                const char* boundary=body;for(;;){current=end;if(*boundary=='['||boundary>=end){if(boundary)current=boundary;break;}
                    boundary=next_line(boundary,end);if(!boundary)break;}
                const char* cursor=body;while(cursor<current){length=0;scan=cursor;
                    while(*scan!='\n'&&*scan!='\r'&&scan<current&&length<=1022)line[length++]=*scan++;line[length]=0;
                    trimmed=line;while(*trimmed&&*trimmed<=' ')++trimmed;if(*trimmed){std::strncpy(assignment,trimmed,1023);
                        if(split_assignment(assignment,key_name,key_value))set(section,key_name,key_value,key_flags);}
                    cursor=next_line(cursor,current);if(!cursor)break;}
            }}if(!current)return;
    }
}
}
