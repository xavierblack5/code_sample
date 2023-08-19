#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "balloc.h"
#include "utils.h"
#include "freelist.h"

typedef struct List{
    void *head;
    void *map;
} *List;

typedef struct {
    int l;
    int u;
    int size;
    void* base;
    FreeList fl;
} *Allocater;



extern Balloc bnew(unsigned int size, int l, int u) {
    int len = 1 + (u - l);
    int e = size2e(size);
    size = e2size(e);
    Allocater a=(Allocater)mmalloc(sizeof(*a)+sizeof(List*)*len);
    a->l = l;
    a->u = u;
    a->size = size;
    a->fl = freelistnew(size, l, u);
    struct List *area = a->fl;
    for(int i = 0; i < len; i++) {
        if(area[i].head)
            a->base=area[i].head; 
    }
    return a;
}

extern void *balloc(Balloc pool, unsigned int size){
    Allocater a = (Allocater) pool;
    int e = size2e(size);
    //Check it f lower than lowest allocation
    if(e < a->l){
        e = a->l;
    }
    //Check if above upper limit 
    return freelistalloc(a->fl,a->base, e, a->u);
}

extern void bfree(Balloc pool, void *mem) {
    Allocater a = (Allocater) pool;
    int e= freelistsize(a->fl, a->base, mem, a->l, a->u);
    freelistfree(a->fl, a->base, mem, e, a->u);
}

extern unsigned int bsize(Balloc pool, void *mem){
    Allocater a = (Allocater) pool;
    return freelistsize(a->fl, a->base, mem, a->l, a->u);
}

extern void bprint(Balloc pool){
    Allocater a = (Allocater) pool;
    char buf[50];
    int n = sprintf(buf, "Lower bound: %d\n", a->l);
    write(1, buf, n);
    n = sprintf(buf, "Upper bound: %d\n", a->u);
    write(1, buf, n);
    n = sprintf(buf, "Size: %d\n", a->size);
    write(1, buf, n);
    n = sprintf(buf, "Block Address: %p\n", a->base);
    write(1, buf, n);
    freelistprint(a->fl, a->size, a->l, a->u);
}