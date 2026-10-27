#include "nfs/resources/localization/region.hpp"
namespace nfs::region {
int selected=0;

int get(){return selected;}

void set(int value){selected=value;}
}
