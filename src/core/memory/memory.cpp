#include "nfs/core/types/kernel.hpp"
#include <cstdlib>
#include <cstring>
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::kernel {
Pool* pools[64]{};Dword default_pool=0;int memory_error=0;OutOfMemory out_of_memory=nullptr;
static Dword address(const void* p){return reinterpret_cast<Dword>(p);}
static Block* at(Dword p){return reinterpret_cast<Block*>(p);}
static FreeBlock* free_node(Block* b){return reinterpret_cast<FreeBlock*>(b);}
static Block* anchor(Pool* p){return &p->anchor.block;}
static void initialize_block(Block* b,int size,Dword flags,Block* previous,Block* next){
    b->flags=static_cast<std::uint16_t>(flags);b->next=next;b->magic=16973;b->size=size;b->previous=previous;
}
static inline void unlink_free_links(Block* block, Block* previous, Block* next) {
    static_cast<volatile FreeBlock*>(free_node(previous))->next_free = next;
    static_cast<volatile FreeBlock*>(free_node(next))->previous_free = previous;
    // The free bit is changed through the high BYTE of the flags WORD.
    auto* high_flags = reinterpret_cast<volatile std::uint8_t*>(&block->flags) + 1;
    *high_flags = static_cast<std::uint8_t>(*high_flags & ~64u);
}

static void unlink_free(Block* block) {
    const volatile FreeBlock* links = free_node(block);
    Block* previous = links->previous_free;
    Block* next = links->next_free;
    unlink_free_links(block, previous, next);
    static_cast<volatile Block*>(block)->magic = 16975;
}
static void insert_free(Pool* p,Block* b){
    int extent=static_cast<int>(address(b->next)-address(b));
    Block* before=anchor(p);Block* after=before;
    Dword lo=address(p->anchor.next_free),hi=address(p->anchor.previous_free);
    Dword middle=lo+static_cast<int>(hi-lo)/2;
    if(middle<address(b)){do{after=free_node(after)->previous_free;}while(address(b)<address(after));before=free_node(after)->next_free;}
    else {do{before=free_node(before)->next_free;}while(address(before)<address(b));after=free_node(before)->previous_free;}
    volatile FreeBlock* links = free_node(b);
    links->next_free = before;
    links->previous_free = after;
    links->block.size = extent;
    static_cast<volatile FreeBlock*>(free_node(after))->next_free = b;
    static_cast<volatile FreeBlock*>(free_node(before))->previous_free = b;
    auto* high_flags = reinterpret_cast<volatile std::uint8_t*>(&b->flags) + 1;
    *high_flags = static_cast<std::uint8_t>(*high_flags | 64u);
    static_cast<volatile Block*>(b)->magic = 16966;
}
int pool_create(Dword id,const char* name,void* storage,int size,Dword alignment,Dword alternate,Dword overhead,bool flag_2000,bool flag_1000,bool locked){
    Dword flags=id|(flag_2000?8192:0)|(flag_1000?4096:0);Dword start=address(storage);
    Dword user=(start+overhead+alternate+155)&~(alternate-1);
    auto* initial=at(user-16);auto* last=at(start+size-48-overhead);auto* first=at(start);
    initialize_block(first,92,flags|32768,nullptr,initial);
    initialize_block(initial,static_cast<int>(address(last)-address(initial)-16),flags,first,last);
    initialize_block(last,0,flags|33024,initial,nullptr);
    auto* p=reinterpret_cast<Pool*>(start+16);pools[id&63]=p;memory_fill(p,0,92);
    auto* output=reinterpret_cast<char*>(p);
    char character;
    do {character=*name++;*output++=character;} while(character);
    p->first=first;p->last=last;first->magic=last->magic=16979;
    p->alignment=alignment;p->flags=flags;p->anchor.block.magic=16979;
    p->anchor.next_free=p->anchor.previous_free=anchor(p);p->anchor.block.size=2147483647;
    p->alternate_alignment=alternate;p->overhead=overhead;p->locked=0;insert_free(p,initial);
    if(locked){lock_init(&p->lock);p->locked=1;}return initial->size;
}
bool pool_destroy(Dword id){auto* p=pools[id&63];if(!p)return false;if(p->locked)lock_destroy(&p->lock);
    memory_fill(p,0,92);pools[id&63]=nullptr;return true;}
int pool_unused(){for(int i=2;i<64;++i)if(!pools[i])return i;return 64;}
Dword memory_initialize(unsigned bytes){register_exit(memory_shutdown);void* storage=std::malloc(bytes);if(!storage)return 0;
    Dword result=pool_create(0,"RAM",storage,bytes,16,128,0,false,false,true);pools[0]->owned=storage;return result;}
void memory_shutdown(){for(int a=0;a<64;++a)for(int b=0;b<64;++b)if(a!=b&&pools[a]==pools[b]&&pools[b])pools[b]=nullptr;
    for(int i=62;i>=0;--i)if(pools[i]){void* owned=pools[i]->owned;pool_destroy(i);if(owned)std::free(owned);pools[i]=nullptr;}}
// The first argument is unused in this image.
Dword allocation_overhead(Dword, Dword flags) {
    return pools[flags & 63]->overhead;
}
void* memory_allocate_aligned(const char*,int size,Dword alignment,Dword offset,Dword flags){
    if(!flags)flags=default_pool;
    Pool* p=pools[flags&63];Dword result=0;if(size<0)return nullptr;
    int retry;do {retry=0;if(p->locked)lock_enter(&p->lock);Dword base_alignment=p->alignment;
        Dword mask=(alignment?alignment:((flags&1024)?p->alternate_alignment:base_alignment))-1;
        int extent=static_cast<int>(Dword(size)+allocation_overhead(0,flags));if(extent<8)extent=8;
        const int needed=static_cast<int>(Dword(extent)+16u+mask);
        Block* block=anchor(p);
        if(!(flags&512)){
            do{block=(flags&256)?free_node(block)->previous_free:free_node(block)->next_free;}while(block->size<needed);
            if(block->magic==16979)block=nullptr;
        }else{
            int threshold=static_cast<int>(Dword(needed)-1u);if(threshold<0)threshold=0;Block* chosen=nullptr;Block* preceding=nullptr;
            do{do{preceding=chosen;block=(flags&256)?free_node(block)->previous_free:free_node(block)->next_free;chosen=preceding;}
                while(static_cast<int>(Dword(block->size)-16u)<=threshold);
                threshold=static_cast<int>(Dword(block->size)-16u);chosen=block;
            }while(block->magic!=16979);block=preceding;
        }
        if(!block){if(out_of_memory)retry=out_of_memory(0,size,flags);}
        else {
            const volatile FreeBlock* free_links = free_node(block);
            Block* previous_free = free_links->previous_free;
            Block* next_free = free_links->next_free;
            const int span = static_cast<volatile Block*>(block)->size;
            unlink_free_links(block, previous_free, next_free);
            Block* next = static_cast<volatile Block*>(block)->next;
            Block* previous = static_cast<volatile Block*>(block)->previous;
            static_cast<volatile Block*>(block)->magic = 16975;
            Dword user=flags&256?((address(block)+offset+span-extent)&~mask)-offset:
                ((address(block)+offset+mask+16)&~mask)-offset;
            Block* allocated=at(user-16);allocated->previous=previous;allocated->next=next;
            if(static_cast<int>(base_alignment+47u)<static_cast<int>(Dword(span)-Dword(needed))){
                if(!(flags&256)){
                    Block* remainder=at((user+extent+base_alignment-1)&~(base_alignment-1));
                    next->previous=remainder;previous->next=allocated;
                    initialize_block(remainder,0,0,allocated,allocated->next);insert_free(p,remainder);allocated->next=remainder;
                }else{
                    next->previous=allocated;allocated->next=next;allocated->previous=block;
                    initialize_block(block,0,0,previous,allocated);insert_free(p,block);
                }
            }else {next->previous=allocated;previous->next=allocated;}
            initialize_block(allocated,size,flags,allocated->previous,allocated->next);result=user;
        }
        if(p->locked)lock_leave(&p->lock);
    }while(retry==1);return reinterpret_cast<void*>(result);
}
void* memory_allocate(const char* name,int size,Dword flags){return memory_allocate_aligned(name,size,0,0,flags);}
int memory_size(const void* memory){return (reinterpret_cast<const Block*>(memory)-1)->size;}
Dword memory_flags(const void* memory){return (reinterpret_cast<const Block*>(memory)-1)->flags;}
std::uint8_t memory_free(void* memory){
    if(!memory)return 1;
    Block* freed=static_cast<Block*>(memory)-1;Pool* p=pools[freed->flags&63];if(p->locked)lock_enter(&p->lock);
    Block* next=freed->next;Block* previous=freed->previous;
    if(previous->flags&16384){unlink_free(previous);freed=previous;previous=freed->previous;freed->next=next;previous->next=freed;next->previous=freed;}
    if(next->flags&16384){unlink_free(next);next=next->next;freed->previous=previous;freed->next=next;next->previous=freed;}
    if(!(previous->flags&49152)){
        int extent=static_cast<int>(Dword(previous->size)+allocation_overhead(0,previous->flags));if(extent<8)extent=8;
        Block* start=at(address(previous)+((p->alignment+15+extent)&~(p->alignment-1)));
        if(freed!=start){start->flags=freed->flags;start->previous=previous;start->next=next;previous->next=start;next->previous=start;freed=start;}
    }
    insert_free(p,freed);if(p->locked)lock_leave(&p->lock);return 1;
}
void* memory_resize(void* memory,int requested){
    Block* b=static_cast<Block*>(memory)-1;
    Dword flags=b->flags;
    Pool* p=pools[flags&63];if(p->locked)lock_enter(&p->lock);Block* next=b->next;
    if(next->flags&16384){unlink_free(next);next=next->next;b->next=next;next->previous=b;}
    int extent=requested;if(extent<8){if(extent==-1)extent=1073741824;else if(extent>=0)extent=8;}
    Dword overhead=allocation_overhead(0,flags);
    Dword alignment_mask=p->alignment-1;
    int capacity=static_cast<int>(address(next)-16-address(b));
    if(capacity<static_cast<int>(Dword(extent)+overhead))extent=requested=static_cast<int>(Dword(capacity)-overhead);
    memory_move(static_cast<char*>(memory)+extent,static_cast<char*>(memory)+b->size,overhead);b->size=requested;
    Block* remainder=at((address(memory)+overhead+Dword(extent)+alignment_mask)&~alignment_mask);
    if(static_cast<int>((address(next)-address(remainder))&4294967280u)>768){initialize_block(remainder,0,0,b,next);insert_free(p,remainder);next->previous=remainder;b->next=remainder;}
    if(p->locked)lock_leave(&p->lock);
    return memory;
}
int memory_largest_free(Dword id) {
    if (!id)
        id = default_pool;
    auto* pool = pools[id & 63];
    Block* current = anchor(pool);
    Block* largest = nullptr;
    int size = 0;
    for (;;) {
        current = static_cast<volatile FreeBlock*>(free_node(current))->next_free;
        const int available = static_cast<int>(
            Dword(static_cast<volatile Block*>(current)->size) - 16u);
        if (available <= size)
            continue;
        if (static_cast<volatile Block*>(current)->magic == 16979)
            break;
        largest = current;
        size = available;
    }
    return largest ? largest->size : 0;
}
}
