#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstring>
namespace nfs::graphics::properties {
// Forward instance-method loop, without added exception cleanup.
void __stdcall construct_elements(void* storage,int stride,int count,InstanceMethod method){auto* p=static_cast<std::uint8_t*>(storage);for(int i=0;i<count;++i){method(p);p+=stride;}}

void __stdcall destroy_elements(void* storage,int stride,int count,InstanceMethod method){auto* p=static_cast<std::uint8_t*>(storage)+static_cast<Dword>(stride)*static_cast<Dword>(count);while(--count>=0){p-=stride;method(p);}}

Dword* value_construct(Dword* value){*value=0;return value;}

Property* property_construct(Property* property){property->name=nullptr;property->count=0;property->values=nullptr;return property;}
// The destructor passes four bytes regardless of allocated value count.
void property_destroy(Property* property){eagl::deallocator(property->values,4);}

char* skip_delimiters(char* text){while(text&&*text&&(*text==';'||*text==','||*text=='='))++text;return text;}

char split_delimiter(char* text,char** end){while(text&&*text&&*text!=';'&&*text!=','&&*text!='=')++text;*end=text;if(!text)return 0;char separator=*text;*text=0;return separator;}

void parse(Parser* parser,const char* input){parser->length=static_cast<int>(std::strlen(input));parser->count=0;parser->entries=nullptr;
    parser->text=static_cast<char*>(eagl::allocator(parser->length+1,nullptr));std::strcpy(parser->text,input);int commas=0,maximum=0,sections=0;
    for(int i=0;i<parser->length;++i){if(parser->text[i]==',')++commas;if(maximum<=commas)maximum=commas;if(parser->text[i]==';'){++sections;commas=0;}}++sections;
    auto* storage=static_cast<Dword*>(eagl::allocator(sections*12+4,"EAGLInternal::Property new[]"));Property* entries=nullptr;
    if(storage){storage[0]=sections;entries=reinterpret_cast<Property*>(storage+1);for(int i=0;i<sections;++i)property_construct(entries+i);}parser->entries=entries;
    for(int i=0;i<sections;++i){Dword* values=static_cast<Dword*>(eagl::allocator((maximum+1)*4,"EAGLInternal::PropertyValue new[]"));if(values)for(int j=0;j<maximum+1;++j)value_construct(values+j);entries[i].values=reinterpret_cast<char**>(values);}
    int count=0,property_index=0;bool assigned=false;char* token=skip_delimiters(parser->text);char separator;
    do{char* end;separator=split_delimiter(token,&end);Property& property=entries[property_index];
        if(separator==0){if(assigned){property.values[count++]=token;property.count=count;++parser->count;++property_index;}}
        else if(separator==','){if(assigned)property.values[count++]=token;}
        else if(separator==';'){if(assigned){property.values[count++]=token;property.count=count;++parser->count;++property_index;}assigned=false;}
        else if(separator=='='&&!assigned){count=0;assigned=true;property.name=token;}
        token=reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(end)+1);
    }while(separator);}

void destroy(Parser* parser){if(parser->entries){Dword* header=reinterpret_cast<Dword*>(parser->entries)-1;for(int i=static_cast<int>(*header)-1;i>=0;--i)property_destroy(parser->entries+i);eagl::deallocator(header,12);}eagl::deallocator(parser->text,parser->length+1);}

std::uint8_t match_property(const Property* property,const char* name,Dword count){return property->count==static_cast<int>(count)&&std::strcmp(name,property->name)==0;}
struct EnumEntry {const char* text;Dword value;unsigned bytes;};
#include "eagl_property_values.inc"
template<unsigned N>static Dword lookup(const char* text,const EnumEntry(&entries)[N],const char* message){for(const auto& entry:entries)if(std::strncmp(text,entry.text,entry.bytes)==0)return entry.value;eagl::report(0,message,text);return 0;}

Dword geo_value(const char* text){return lookup(text,geo_entries,"INTERNAL ERROR: Invalid GeoPrimState function parameter '%s'\n");}

Dword pc_value(const char* text){return lookup(text,pc_entries,"INTERNAL ERROR: Invalid GeoPrimState value %s\n");}

Dword tar_api_value(const char* text){return lookup(text,tar_api_entries,"INTERNAL ERROR: Invalid TAR api function parameter '%s'\n");}

Dword tar_value(const char* text){return lookup(text,tar_entries,"INTERNAL ERROR: Invalid TAR value %s\n");}
}
