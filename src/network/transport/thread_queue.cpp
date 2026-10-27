#include "nfs/network/transport/thread_queue.hpp"
namespace nfs::lifecycle::construction::network::queue {
static Dword* pointer(Dword value){return reinterpret_cast<Dword*>(value);}
static void enter(Dword* object){if(object[15])EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(object+9));}
static void leave(Dword* object){if(object[15])LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(object+9));}
static bool equal(Dword* object,Dword first,Dword second,Dword* comparer){using Equal=Dword(__thiscall*)(Dword*,Dword,Dword);Dword* receiver=comparer?comparer:object;
    return reinterpret_cast<const Equal*>(receiver[0])[comparer?0:2](receiver,first,second)==1;}
static void copy_node(Dword* object,Dword output,Dword* node){using Size=Dword(__thiscall*)(Dword*,Dword);using Copy=void(__thiscall*)(Dword*,Dword,Dword,Dword);
    Dword methods=object[0];Dword size=reinterpret_cast<const Size*>(methods)[4](object,node[4]);reinterpret_cast<const Copy*>(methods)[8](object,output,node[4],size);}
static Dword *sorted(Dword *object, Dword direction, Dword context, Dword *comparer) {
    Dword *head = pointer(object[5]);
    head[2] = 0;
    for (Dword *node = pointer(head[1]); node; node = pointer(node[1])) {
        Dword *previous = nullptr;
        Dword *position = head;
        while (position) {
            using Compare = Dword(__thiscall *)(Dword *, Dword, Dword, Dword);
            using External = Dword(__thiscall *)(Dword *, Dword, Dword, Dword);
            const Dword result = comparer
                ? reinterpret_cast<const External *>(comparer[0])[0](
                      comparer, node[4], position[4], context)
                : reinterpret_cast<const Compare *>(object[0])[1](
                      object, node[4], position[4], context);
            if (result == (direction == 3 ? 4294967295u : 1u)) {
                node[2] = reinterpret_cast<Dword>(position);
                if (previous)
                    previous[2] = reinterpret_cast<Dword>(node);
                else
                    head = node;
                break;
            }
            previous = position;
            position = pointer(position[2]);
        }
        if (!position) {
            node[2] = 0;
            previous[2] = reinterpret_cast<Dword>(node);
        }
    }
    return head;
}

Dword count(Dword* object){enter(object);Dword result=object[7];leave(object);return result;}

Dword selected_count(Dword* object){enter(object);Dword result=object[8];leave(object);return result;}

Dword consume_changed(Dword* object){enter(object);Dword result=object[16];object[16]=0;leave(object);return result;}

Dword mark_changed(Dword* object){enter(object);object[16]=1;leave(object);return 1;}

void visit(Dword* object,Dword direction,Dword context,Dword* visitor,Dword order_context,Dword* comparer){enter(object);enter(object);Dword total=object[7];leave(object);
    if(total&&direction<4){Dword* node=direction==1?pointer(object[6]):direction>=2?sorted(object,direction,order_context,comparer):pointer(object[5]);Dword index=0;
        while(node&&(direction<2||static_cast<int>(index)<static_cast<int>(object[7]))){using Visit=Dword(__thiscall*)(Dword*,Dword,Dword,Dword,Dword);Dword* receiver=visitor?visitor:object;
            if(!reinterpret_cast<const Visit*>(receiver[0])[visitor?0:3](receiver,node[4],index,node[3],context))break;
            node=pointer(node[direction==1?0:direction>=2?2:1]);++index;}}
    leave(object);}

void unlink(Dword* object,Dword* node){if(node){if(node[3])--object[8];if(object[5]==reinterpret_cast<Dword>(node))object[5]=node[1];if(object[6]==reinterpret_cast<Dword>(node))object[6]=node[0];
    if(node[1])pointer(node[1])[0]=node[0];if(node[0])pointer(node[0])[1]=node[1];--object[7];object[16]=1;}}

Dword* find(Dword* object,Dword key,Dword* comparer){if(object[7])for(Dword* node=pointer(object[5]);node;node=pointer(node[1]))if(equal(object,key,node[4],comparer))return node;return nullptr;}

void select(Dword* object,Dword* node,Dword mode){if(object[3]==4294967295u)return;
    if((mode==0&&node[3])||mode==2){if(node[3]){node[3]=0;--object[8];object[16]=1;}return;}
    if(mode>1||(mode==1&&node[3]))return;
    if(object[3]&&static_cast<int>(object[8])>=static_cast<int>(object[3])){for(Dword* item=pointer(object[5]);item;item=pointer(item[1]))item[3]=0;object[8]=0;}
    ++object[8];node[3]=1;object[16]=1;}

Dword set_selected(Dword* object,Dword key,Dword mode,Dword* comparer){if(object[3]==4294967295u)return 0;enter(object);Dword* node=find(object,key,comparer);
    if(node){select(object,node,mode);leave(object);return 1;}leave(object);return 0;}

Dword replace(Dword* object,Dword key,Dword value,Dword* replacer){enter(object);Dword* node=find(object,key,nullptr);if(node){Dword selected=node[3];
    if(replacer){using Replace=void(__thiscall*)(Dword*,Dword,Dword);reinterpret_cast<const Replace*>(replacer[0])[0](replacer,value,node[4]);}
    else{using Size=Dword(__thiscall*)(Dword*,Dword);Dword incoming=reinterpret_cast<const Size*>(object[0])[4](object,value);Dword previous=reinterpret_cast<const Size*>(object[0])[4](object,node[4]);
        if(static_cast<int>(previous)<static_cast<int>(incoming)){using Destroy=void(__thiscall*)(Dword*,Dword);reinterpret_cast<const Destroy*>(object[0])[7](object,node[4]);using Clone=Dword(__thiscall*)(Dword*,Dword,Dword);
            node[4]=reinterpret_cast<const Clone*>(object[0])[6](object,value,reinterpret_cast<Dword>("listnode"));}
        else{using Copy=void(__thiscall*)(Dword*,Dword,Dword,Dword);Dword methods=object[0];Dword size=reinterpret_cast<const Size*>(methods)[4](object,value);
            reinterpret_cast<const Copy*>(methods)[8](object,node[4],value,size);}}
    node[3]=selected;object[16]=1;leave(object);return 1;}leave(object);return 0;}

Dword erase(Dword* object,Dword key,Dword* comparer){enter(object);Dword* node=find(object,key,comparer);if(node){unlink(object,node);leave(object);using Destroy=void(__thiscall*)(Dword*,Dword);
    reinterpret_cast<const Destroy*>(object[0])[7](object,node[4]);gui::application_release(node);return 1;}leave(object);return 0;}

Dword copy_matching(Dword* object,Dword key,Dword output,Dword* comparer){if(!key)return 0;enter(object);Dword* node=find(object,key,comparer);if(node){if(output)copy_node(object,output,node);leave(object);return 1;}leave(object);return 0;}

Dword copy_selected(Dword* object,Dword output){if(!count(object)||object[3]!=1)return 0;enter(object);for(Dword* node=pointer(object[5]);node;node=pointer(node[1]))if(node[3]){if(output)copy_node(object,output,node);leave(object);return 1;}leave(object);return 0;}

Dword copy_nth(Dword* object,Dword order_context,int index,Dword direction,Dword output,Dword* comparer,Dword filter_context,Dword* filter){if(index<0||!count(object))return 0;enter(object);
    if(direction<4){Dword* node=direction==1?pointer(object[6]):direction>=2?sorted(object,direction,order_context,comparer):pointer(object[5]);int at=0;
        while(node&&(direction<2||at<static_cast<int>(object[7]))){using Filter=Dword(__thiscall*)(Dword*,Dword,Dword);
            if((!filter||reinterpret_cast<const Filter*>(filter[0])[0](filter,node[4],filter_context))&&at==index){if(output)copy_node(object,output,node);leave(object);return 1;}
            node=pointer(node[direction==1?0:direction>=2?2:1]);++at;}}
    leave(object);return 0;}

Dword take(Dword* object,Dword key,Dword output){if(count(object)){enter(object);Dword* node=key?find(object,key,nullptr):pointer(object[5]);if(node){if(output)copy_node(object,output,node);unlink(object,node);leave(object);
    using Destroy=void(__thiscall*)(Dword*,Dword);reinterpret_cast<const Destroy*>(object[0])[7](object,node[4]);gui::application_release(node);return 1;}leave(object);}return 0;}
}
