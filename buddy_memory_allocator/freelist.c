#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "freelist.h"
#include "utils.h"
#include "bitmap.h"


typedef struct List{
    void *head;
    BitMap *map;
} *List;



// Use size to allocate freeblocks from and add to each freelist
extern FreeList freelistnew(unsigned int size, int l, int u) {
    int e = size2e(size);
    int len = (u - l);
    int order;
    struct List *fl = mmalloc(sizeof(List)*len);
    void* base = mmalloc(size);
    unsigned int offset;
    int max = u;
    for(int i = 0; i <= len; i++) {
        fl[i].head = 0;
        order = u - i;
        //Saving allocation of bitmaps of too large orders
        if(e >= order) {
            fl[i].map = bitmapnew(size, order);
            if(order == max) {
                fl[i].head = base;
                offset = e2size(max);
            }
        }
        else {
            fl[i].map = 0;
            max--;
        }
    }

    void* next = base + offset;
    //Set first pointer in block to 0
    memset(base, 0, sizeof(next));
    //Setup list if bigger than size of one block
    if(offset < size) {
        //Set first start of first block to address of second
        *(uint64_t*)base = next;
        //while loop to set first part of block to address of next block
        for(int i = 2; i*offset < size; i++) {
            *(uint64_t*)next = next + offset;
            next = next + offset;
        }
        //Set beginning of last block to 0 indicating end
        memset(next, 0, sizeof(base));
    }
    return fl;
}

//e is size and l is lowest possible size. 
extern void *freelistalloc(FreeList f, void *base, int e, int u) {
    struct List *fl= (List) f;
    int index = (e - u) * -1;
    // index >= 0 && 
    // Have a block of the right size
    if(fl[index].head) {
        void* r = fl[index].head;
        void* next = *(uint64_t*)fl[index].head;
        fl[index].head = next;
        bitmapset(fl[index].map, base, r, e);
        return r;
    } else {
        // Get address of a free block of double the size of e
        void* e2 = freelistalloc(f,base, e + 1, u);
        unsigned int offset = e2size(e);
        //Get address of beginning of second half of 2e
        void* next = e2 + offset;
        //Set second block beginning to 0
        memset(next, 0, sizeof(base));
        //Set head of list to second half as first half will be allocated
        fl[index].head = next;
        bitmapset(fl[index].map, base, e2, e);
        return e2;
    }
    
}
extern void freelistfree(FreeList f, void *base, void *mem, int e, int u) {
    if(e > u){
        return;
    }
    struct List *fl= (List) f;
    int index = (e - u) * -1;
    void *next = fl[index].head;
    void* prev = 0;
    void* prevp = 0;
    //Freelist is empty add block to front
    if(!next){
        fl[index].head = mem;
        memset(mem, 0, sizeof(base));
        return;
    }
    int isleft = buddytst(base, mem, e);
    // Block to free is smaller than head so add to beginning and set beginning of it to the old head
    if(mem < next) {
        *(uint64_t*)mem = next;
        fl[index].head = mem;
        // Coalesce Buddies
        if(next == buddyset(base, mem, e)){
            if(e != u)
                fl[index].head = *(uint64_t*)next;
            freelistfree(f, base, mem, e + 1, u);
            bitmapclr(fl[index].map, base, mem, e);
            return;
        }
    } else {
        prev = next;
        next = *(uint64_t*)fl[index].head;
        //Block is in the list but not the end
        while(next){
            if(mem < next){
                *(uint64_t*)mem = next;
                *(uint64_t*)prev = mem;
                break;
            }
            prevp = prev;
            prev = next;
            next = *(uint64_t*)next;
        }
    }  
    //Block is at the end of the list
    if(next == 0) {
        *(uint64_t*)prev = mem;
        memset(mem, 0, sizeof(base));
        //Coalesce with left buddy as prev
        if(prev == buddyclr(base, mem, e)){
            //Left buddy is head
            if(prev == fl[index].head){
                fl[index].head = 0;
            } else {
                memset(prevp, 0, sizeof(base));
            }
            freelistfree(f, base, prev, e + 1, u);
            bitmapclr(fl[index].map, base, mem, e);
        }
        return;
    }
    
    //Block freed is left buddy
    if(isleft && next == buddyset(base, mem, e)) {
        //Buddy is on free list so need to coalesce
            // Get rid of buddies from current fl and then free left buddy address from list above
            *(uint64_t*)prev = *(uint64_t*)next;
    } else if (!isleft){
        void* lbud = buddyclr(base, mem, e);
        //Left buddy on list and is head
        if(prev == lbud && prev == fl[index].head){
            fl[index].head = next;
            freelistfree(f, base, lbud, e + 1, u);
            bitmapclr(fl[index].map, base, mem, e);
            return;
        } //Left buddy on list but not head
        else if(prev == lbud){
            *(uint64_t*)prevp = next;
            freelistfree(f, base, lbud, e + 1, u);
            bitmapclr(fl[index].map, base, mem, e);
            return;
        }
    }
}

extern int freelistsize(FreeList f, void *base, void *mem, int l, int u) {
    struct List *fl= (List) f;
    int len = (u - l);
    int order;
    for(int i = len; i >= 0; i--) {
        order = u - i;
        if(bitmaptst(fl[i].map, base, mem, order))
            break;
    }
    return order;
}

extern void freelistprint(FreeList f, unsigned int size, int l, int u) {
    struct List *fl= (List) f;
    char buf[50];
    int len = (u - l);
    int order, n;
    for(int i = 0; i <= len; i++) {
        order = u - i;
        int n = sprintf(buf, "Freelist %d-order bitmap: \n", order);
        write(1, buf, n); 
        if(fl[i].map){
            bitmapprint(fl[i].map, size, order);
        } else {
            n = sprintf(buf, "No Bitmap\n");
            write(1, buf, n);
        }
        int bnum = 0;
        if(fl[i].head != 0) {
            void *next = *(uint64_t*)fl[i].head;
            n = sprintf(buf, "Head Pointer: %p\n", fl[i].head);
            write(1, buf, n);
            bnum = 1;
            while(next){
                n = sprintf(buf, "Next Pointer: %p\n", next);
                write(1, buf, n);
                next = *(uint64_t*)next;
                bnum++;
            }
        }
        n = sprintf(buf, "Total Free Blocks of Order %d: %d\n", order, bnum);
        write(1, buf, n);
    }
    n = sprintf(buf, "\n");
    write(1, buf, n);
}


