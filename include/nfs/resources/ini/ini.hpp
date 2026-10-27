#pragma once
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::ini {
using gui::Dword;
struct Key {const gui::Destroy* methods;gui::PoolString* name;gui::String value;};
struct Section {const gui::Destroy* methods;gui::PoolString* name;gui::Container keys;};
struct Document {const gui::Destroy* methods;gui::PoolString* name;gui::PoolString* directory;gui::Container sections;Dword auto_save;};
struct CachedDocument {Document document;CachedDocument* next;CachedDocument* previous;int references;};
struct Reader {const gui::Destroy* methods;std::uint8_t modified;std::uint8_t unknown_5[3];CachedDocument* document;};
static_assert(sizeof(Key)==28&&sizeof(Section)==24&&sizeof(Document)==32&&sizeof(CachedDocument)==44&&sizeof(Reader)==12);
static_assert(offsetof(Document,sections)==12&&offsetof(CachedDocument,next)==32);
Document* construct(Document*,const char*,const char*);void clear(Document*);
void parse(Document*,const char*,int,Dword,Dword);
const char* parse_section(Section*,const char*,const char*,Dword);void rename(Section*,const char*);
Section* add_section(Document*,const char*,Dword);Section* find_section(Document*,const char*);void remove_section(Document*,const char*);
Key* set(Section*,const char*,const char*,Dword);Key* find(Section*,const char*);void remove(Section*,const char*);
const char* next_line(const char*,const char*);Dword split_assignment(const char*,char*,char*);
Dword save(Document*);Dword read_only(Document*);
extern CachedDocument* cached;extern Dword load_pool;
CachedDocument* acquire(const char*,std::uint8_t);void release(CachedDocument*);void flush_cache();
Reader* reader_construct(Reader*,const char*,std::uint8_t);void reader_clear(Reader*);
Section* reader_section(Reader*,const char*);Key* reader_key(Reader*,const char*,const char*);
std::uint8_t get_string(Reader*,const char*,const char*,char*,Dword);bool get_int(Reader*,const char*,const char*,int*);
bool get_float(Reader*,const char*,const char*,float*);std::uint8_t get_bool(Reader*,const char*,const char*,Dword*);
std::uint8_t get_short(Reader*,const char*,const char*,short*);
std::uint8_t set_int(Reader*,const char*,const char*,int);std::uint8_t set_word(Reader*,const char*,const char*,Dword);
std::uint8_t set_float(Reader*,const char*,const char*,float,int);
std::uint8_t set_string(Reader*,const char*,const char*,const char*);
}
