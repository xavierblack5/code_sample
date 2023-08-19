#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <sys/mman.h>

#include "utils.h"


extern void *mmalloc(size_t size){
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
}

extern unsigned int e2size(int e) {
    return (unsigned int) 1<<e;
}
extern int size2e(unsigned int size) {
    int e = 1;
    int power = 2;
    while(power < size) {
        power*=2;
        e+=1;
    }
 
    return e;
}

extern void bitset(unsigned char *p, int bit) {
    *p = *p | (1 << bit);
}
extern void bitclr(unsigned char *p, int bit) {
    *p = *p & ~(1 << bit);
}
extern void bitinv(unsigned char *p, int bit) {
    *p = *p ^ (1 << bit);
}
extern int bittst(unsigned char *p, int bit) {
    int r = *p & (1 << bit);
    return r;
}

//Get right buddy
extern void *buddyset(void *base, void *mem, int e){
    void* buddy = buddyinv(base, mem, e);
    return (mem > buddy ? mem : buddy);
}
//Get left buddy
extern void *buddyclr(void *base, void *mem, int e){
    void* buddy = buddyinv(base, mem, e);
    return (mem < buddy ? mem : buddy);
}
//Get opposite buddy
extern void *buddyinv(void *base, void *mem, int e){
    //XOR
    unsigned int size = e2size(e);
    return (void*)((uint64_t)mem ^ size);
}

//Tell if right or left buddy 1 = left and 0 = right
extern int buddytst(void *base, void *mem, int e){
    void* buddy = buddyinv(base, mem, e);
    return (mem < buddy ? 1 : 0);
}


