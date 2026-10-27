#include "nfs/resources/files/file_cache.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::files::cache::application {
Paths paths{".",{},".\\"};
std::uint8_t disabled=0,active=0,media_available=0;
Dword discover(){
    UINT previous_mode=SetErrorMode(32768);media_available=0;HKEY key;
    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE,"Software\\EA Games\\Need For Speed Hot Pursuit 2",0,131097,&key)==0||
       RegOpenKeyExA(HKEY_LOCAL_MACHINE,"Software\\EA Games\\Need For Speed Poursuite Infernale 2",0,131097,&key)==0){
        char buffer[500]{};DWORD type=1,size=500;
        if(RegQueryValueExA(key,"Install Dir",nullptr,&type,reinterpret_cast<BYTE*>(buffer),&size)==0){buffer[499]=0;
            if(GetLongPathNameA(buffer,buffer,500)){buffer[499]=0;std::strncpy(paths.installation,buffer,500);paths.installation[499]=0;size=500;
                if(RegQueryValueExA(key,"Installed From",nullptr,&type,reinterpret_cast<BYTE*>(buffer),&size)==0){buffer[499]=0;
                    if(GetLongPathNameA(buffer,buffer,500)){buffer[499]=0;std::strncpy(paths.media,buffer,500);paths.media[499]=0;RegCloseKey(key);
                        char filename[260];std::sprintf(filename,"%sAudio\\Music\\Track0.asf",paths.media);FILE* file=std::fopen(filename,"rb");
                        if(file){std::fclose(file);media_available=1;SetErrorMode(previous_mode);return 1;}
                        for(std::uint8_t drive='D';;){char root[12];std::sprintf(root,"%c:\\",drive);
                            if(GetDriveTypeA(root)==DRIVE_CDROM){std::sprintf(filename,"%c:\\Audio\\Music\\Track0.asf",drive);file=std::fopen(filename,"rb");
                                if(file){std::sprintf(paths.media,"%c:\\",drive);if(drive>'Z'){SetErrorMode(previous_mode);return 0;}
                                    std::fclose(file);media_available=1;SetErrorMode(previous_mode);return 1;}}
                            ++drive;if(drive>'Z'){SetErrorMode(previous_mode);return 0;}
                        }
                    }
                }
            }
        }
    }
    RegCloseKey(key);SetErrorMode(previous_mode);return 0;
}
Dword initialize(){
    if(!active){if(!media_available)return 0;char root[4]{paths.installation[0],paths.installation[1],paths.installation[2],0};
        DWORD sectors,sector_bytes,free_clusters,clusters;
        if(!GetDiskFreeSpaceA(root,&sectors,&sector_bytes,&free_clusters,&clusters)||std::uint64_t(free_clusters)*sector_bytes*sectors<314572800){active=0;return 5;}
        Settings values;values.size=40;get_settings(&values);
        if(!disabled){set_directory(paths.media);values.destination=paths.installation;values.byte_limit=629145600;values.source=paths.media;values.free_space_limit=419430400;
            if(set_settings(&values)){cache::initialize();char filename[520];std::sprintf(filename,"%sen-us_filelist.txt",paths.media);cache::enqueue_list(filename);}
            active=1;}
    }
    return 0;
}
void shutdown(){if(active){cache::shutdown();active=0;}}
Dword account_existing(const char* path){if(!active)return 0;return cache::account_existing(path);}
void enqueue(const Input* entries,int count){if(active)cache::enqueue(entries,count);}
int enqueue_list(const char* filename,Dword){if(!active)return 0;return cache::enqueue_list(filename);}
void start(){if(active)cache::start();}
void pause(){if(active)cache::pause();}
char* directory(){return paths.installation;}
}
