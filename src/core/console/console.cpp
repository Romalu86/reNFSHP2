#include "nfs/core/console/console.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace nfs::kernel::print {

int check_console_write(const char*,...){return 0;}
static void __cdecl console_destination(int,const char* text){check_console_write("%s",text);}
// This function appends until a newline, then emits the entire accumulated text. Its nominal
// backing-buffer capacity is not established; no bounds policy is added.
static char debugger_text[1024]{};
static void __cdecl debugger_destination(int,const char* text){std::strcat(debugger_text,text);
    if(std::strchr(debugger_text,'\n')){OutputDebugStringA(debugger_text);debugger_text[0]=0;}}
static HANDLE log_file=INVALID_HANDLE_VALUE;
static void __cdecl file_destination(int,const char* text){DWORD size=static_cast<DWORD>(std::strlen(text)),written;
    if(log_file!=INVALID_HANDLE_VALUE&&(!WriteFile(log_file,text,size,&written,nullptr)||written!=size))OutputDebugStringA("PRINTFILE_print: write failed.\n");}
Device devices[8]={{},{"console",console_destination,1},{"debugger",debugger_destination,1},{"file log",file_destination,0},{},{},{},{}};
Channel channels[64]={{"all",2},{"real",2},{"alert",3},{"general",2},{"memory",2},{"connection",2},{"graphics",2},{"file",2},
    {"input",2},{"system",2},{"3d",2},{"font",2},{"codec",2},{"math",2},{"eagl",2},{"capture",2}};
static bool initialized=false;
void initialize(){if(!initialized){initialized=true;for(auto& d:devices)d.flags&=~1u;for(auto& c:channels)c.flags&=~1u;
    for(int i=4;i<8;++i){devices[i].write=nullptr;devices[i].flags=0;}
    for(int i=14;i<64;++i){channels[i].name=nullptr;channels[i].flags&=~2u;}
    set_channel(2,1);set_device(1,1);}}
void reset(){if(initialized)initialized=false;}
void write_v(int channel,const char* format,va_list arguments){char text[8192];if(!initialized)initialize();
    if(channels[channel].flags&1){std::vsprintf(text,format,arguments);for(auto& d:devices)if((d.flags&1)&&d.write)d.write(channel,text);}}
void write(int channel,const char* format,...){va_list arguments;va_start(arguments,format);write_v(channel,format,arguments);va_end(arguments);}
void device_flag(int id,Dword value){devices[id].flags^=(devices[id].flags^value)&1;}
void set_device(int id,Dword value){if(devices[id].write){if(!initialized)initialize();device_flag(id,value);
    const char* setting=std::getenv("PRINT_DEVICE");if(setting)parse_devices(setting);}}
void lower_copy(const char* input,char* output,int capacity){Dword i=0;
    for(;;){Dword last=static_cast<Dword>(capacity-1),length=static_cast<Dword>(std::strlen(input));if(length<last)last=length;
        if(last<i){output[i]=0;return;}char c=input[i];output[i]=(c>='A'&&c<='Z')?static_cast<char>(c+32):c;++i;}}
int prefix_length(const char* name,const char* token){int matched=0;while(*name){if(!*token)return matched;
    char a=*name++,b=*token++;if(a>='A'&&a<='Z')a+=32;if(b>='A'&&b<='Z')b+=32;if(a!=b)return matched;++matched;}return matched;}
int find_device(const char* name){int found=-1,best=0;for(int i=0;i<8;++i)if(devices[i].name){int length=prefix_length(devices[i].name,name);if(best<length){best=length;found=i;}}return found;}
void parse_devices(const char* setting){char text[256];lower_copy(setting,text,256);char* token=std::strtok(text," ,");
    while(token){bool disabled=*token=='-';int id=find_device(token+(disabled?1:0));if(id>=0)device_flag(id,disabled?0:1);token=std::strtok(nullptr," ,");}}
void register_device(int id,const char* name,Destination destination){if(!initialized)initialize();devices[id].name=name;devices[id].write=destination;
    const char* setting=std::getenv("PRINT_DEVICE");if(setting)parse_devices(setting);}
void channel_flag(int id,int value){if(id==0){for(auto& c:channels)c.flags^=(c.flags^Dword(value!=0))&1;return;}
    if(id==1){for(auto& c:channels)if(c.flags&2)c.flags^=(c.flags^Dword(value!=0))&1;return;}
    channels[id].flags^=(channels[id].flags^Dword(value!=0))&1;}
void set_channel(int id,int value){if(!initialized)initialize();channel_flag(id,value);
    const char* setting=std::getenv("PRINT_CHANNEL");if(setting)parse_channels(setting);}
int find_channel(const char* name){int found=-1,best=0;for(int i=0;i<64;++i)if(channels[i].name){int length=prefix_length(channels[i].name,name);if(best<length){best=length;found=i;}}return found;}
void parse_channels(const char* setting){char text[256];lower_copy(setting,text,256);char* token=std::strtok(text," ,;");
    while(token){bool disabled=*token=='-'||*token=='!';int id=find_channel(token+(disabled?1:0));if(id>=0)channel_flag(id,disabled?0:1);token=std::strtok(nullptr," ,;");}}
Dword enabled(int id){if(!initialized)initialize();return channels[id].flags&1;}
void register_channel(int id,const char* name){if(!initialized)initialize();channels[id].name=name;
    const char* setting=std::getenv("PRINT_CHANNEL");if(setting)parse_channels(setting);}
void direct(int device,int channel,const char* text){if(!initialized)initialize();if(devices[device].write)devices[device].write(channel,text);}
void open_log(const char* path){char name[256],message[256];std::sprintf(name,"%s",path);
    log_file=CreateFileA(name,GENERIC_READ|GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE,nullptr);
    if(log_file==INVALID_HANDLE_VALUE){std::sprintf(message,"Failed to open %s for writing.\n",name);OutputDebugStringA(message);return;}
    register_exit(close_log);devices[3].flags|=1;}
void close_log(){if(log_file!=INVALID_HANDLE_VALUE){CloseHandle(log_file);log_file=INVALID_HANDLE_VALUE;devices[3].flags&=~1u;}}
}
