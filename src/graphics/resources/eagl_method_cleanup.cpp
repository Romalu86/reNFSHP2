#include "nfs/graphics/resources/eagl_cleanup.hpp"
namespace nfs::graphics::resources {
static Dword* pointer(Dword value){return reinterpret_cast<Dword*>(value);}
// The second step uses the header read before the zero skip.
Dword* find_technique(int index,Dword* command){if(!command||index<0)return nullptr;int found=0;while(found!=index){Dword kind=command[0]>>16;
    if(kind==17||kind==16||kind==65||kind==64||kind==48||kind==47||kind==67||kind==66){command+=command[0]&65535;++found;}
    Dword header=command[0];if(!header)++command;command+=header&65535;}return command;}

void normalize_command(Dword* command){Dword kind=command[0]>>16,size=command[0]&65535;
    if(kind==19||kind==22){Dword replacement=kind;if(vertex_program_mode)replacement=kind==22?23:20;else if(pixel_program_mode)replacement=kind==22?24:21;command[0]=size|(replacement<<16);}
    if(kind==37||kind==40){Dword replacement=kind;if(vertex_program_mode)replacement=kind==40?41:38;else if(pixel_program_mode)replacement=kind==40?42:39;command[0]=size|(replacement<<16);}}

void release_program_data(Dword data){auto* header=pointer(data-16);eagl::deallocator(header,header[0]);}
static void clear_value(Dword* command,bool nested){Dword kind=command[0]>>16;
    if(kind==4){Dword value=command[4];if(value&&value!=4294967295u){int count=static_cast<int>(command[8]);if(count<1){Dword handle=pointer(value)[1];release_vertex(&handle,0);}
        else for(int i=0;i<count;++i){Dword handle=pointer(command[4])[1+i*2];release_vertex(&handle,0);}eagl::deallocator(pointer(command[4]),8);}command[4]=0;}
    else{if(kind==5||kind==76){Dword handle=command[4];release_vertex(&handle,0);command[4]=4294967295u;}
        if(kind==75){Dword handle=command[4];command[4]=0;destroy_cached_vertex(&handle);release_program_data(command[10]);}
        else if(!nested&&kind==7)release_index(command+3);}}

void clear_method(Dword* method,Dword technique){Dword* command=find_technique(static_cast<int>(technique),method+1);while(command[0]){Dword step=command[0]&65535,kind=command[0]>>16;
    clear_value(command,false);if(kind==64||kind==65){Dword* child=command+1;while(child[0]){Dword child_step=child[0]&65535;clear_value(child,true);child+=child_step;}}command+=step;}}
static void clear_variant_value(Dword* command,bool nested){Dword kind=command[0]>>16;if(kind==4&&command[9]==1){Dword count=command[8];command[9]=0;bool forward=static_cast<int>(count)>0;if(!forward)count=0u-count;
    Dword handle=pointer(command[4])[1];release_vertex(&handle,0);pointer(command[4])[1]=0;
    for(int i=1;i<static_cast<int>(count);++i)if(forward){handle=pointer(command[4])[1+i*2];release_vertex(&handle,0);pointer(command[4])[1+i*2]=0;}
    eagl::deallocator(pointer(command[4]),8);command[4]=0;}
    else if(kind==75&&command[9]==1){Dword handle=command[4];command[9]=0;destroy_cached_vertex(&handle);command[4]=4294967295u;command[10]=4294967295u;}(void)nested;}

void clear_method_variant(Dword* method,Dword technique){Dword* command=find_technique(static_cast<int>(technique),method+1);while(command[0]){Dword step=command[0]&65535,kind=command[0]>>16;normalize_command(command);
    if(kind==64||kind==65){Dword* child=command+1;while(child[0]){Dword child_step=child[0]&65535;clear_variant_value(child,true);child+=child_step;}}
    clear_variant_value(command,false);command+=step;}}

void method_destroy(Dword* method){if(method[3]){int count=static_cast<int>(technique_count(pointer(method[3])));if(method[9]){for(int i=0;i<count;++i){Dword technique=technique_at(pointer(method[3]),i);
    if(!method[6]){if(!i)clear_method(pointer(method[9]),technique);else clear_method_variant(pointer(method[9]),technique);}}method[9]=0;}}
    Dword* dictionary=pointer(method[3]);if(dictionary&&!method[4]){dictionary_destroy(dictionary);eagl::deallocator(dictionary,36);method[3]=0;}
    Dword data=method[1];if(data)eagl::deallocator(pointer(data-4),pointer(data-4)[0]);method[1]=0;}
}
