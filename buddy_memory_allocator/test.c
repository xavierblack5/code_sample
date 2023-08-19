#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

#include "balloc.h"
#include "utils.h"
#include "freelist.h"
#include "wrapper.h"
#include "deq.h"

int main() {
    
    // char buf[50];
    // int n;
    // int e = size2e(415);
    // n = sprintf(buf, "Size2e test: %d\n", e);
    // write(1, buf, n);
    // unsigned int size = e2size(2);
    // n = sprintf(buf, "e2size test: %u\n", size);
    // write(1, buf, n);
    // int x = (5 - 6) * -1;
    //  n = sprintf(buf, "Negative index: %d\n", x);
    // write(1, buf, n);
    // n = sprintf(buf, "Bnew test:\n");
    // write(1, buf, n);

    //Simple test for deallocating
    // Balloc bp = bnew(64,4,5);
    // bprint(bp);
    // void* mem1 = balloc(bp, 32);
    // bprint(bp);
    // void* mem2 = balloc(bp,16);
    // bprint(bp);
    // bfree(bp, mem2);
    // bprint(bp);
    // bfree(bp, mem1);
    // bprint(bp);
    

    //Bigger dealloc test
    // Balloc bp = bnew(4096,4,12);
    // bprint(bp);
    // void* mem1 = balloc(bp, 1024);
    // bprint(bp);
    // void* mem2 = balloc(bp, 16);
    // void* mem3 = balloc(bp, 512);
    // bprint(bp);
    // bfree(bp, mem3);
    // bprint(bp);
    // bfree(bp, mem2);
    // bprint(bp);
    // bfree(bp, mem1);
    // bprint(bp);

    // // Complivated deallocation
    // Balloc bp = bnew(1024,4,10);
    // // bprint(bp);
    // void* mem1 = balloc(bp, 16);
    // void* mem2 = balloc(bp, 16);
    // void* mem3 = balloc(bp, 16);
    // void* mem4 = balloc(bp, 16);
    // void* mem5 = balloc(bp, 16);
    // void* mem6 = balloc(bp, 16);
    // void* mem7 = balloc(bp, 16);
    // void* mem8 = balloc(bp, 16);
    // // bprint(bp);
    // bfree(bp, mem1);
    // bprint(bp);
    // bfree(bp, mem7);
    // bprint(bp);
    // bfree(bp, mem5);
    // bprint(bp);
    // bfree(bp, mem3);
    // bprint(bp);
    // // n = sprintf(buf, "Mem2 Pointer: %p\n", mem2);
    // // write(1, buf, n);
    // bfree(bp, mem2);
    // bprint(bp);
    // // n = sprintf(buf, "Mem6 Pointer: %p\n", mem6);
    // // write(1, buf, n);
    // bfree(bp, mem6);
    // bprint(bp);
    // bfree(bp, mem4);
    // bprint(bp);
    // bfree(bp, mem8);
    // bprint(bp);
    



    //Test bsize
    // int size1 = bsize(bp, mem1);
    // int size2 = bsize(bp, mem2);
    // int size3 = bsize(bp, mem3);
    // n = sprintf(buf, "Size of first malloc: %d\n", size1);
    // write(1, buf, n);
    // n = sprintf(buf, "Size of second malloc: %d\n", size2);
    // write(1, buf, n);
    // n = sprintf(buf, "Size of second malloc: %d\n", size3);
    // write(1, buf, n);
    
    //Large allocation
    // Balloc bp = bnew(8192,4,11);
    // bprint(bp);
    // balloc(bp, 32);
    // bprint(bp);
    // balloc(bp,16);
    // bprint(bp);

    // Buddy inv test
    // void* bud = buddyinv(mem, mem, 4);
    // n = sprintf(buf, "Left Buddy: %p\n", mem);
    // write(1, buf, n);
    // n = sprintf(buf, "Right Buddy: %p\n", bud);
    // write(1, buf, n);

    //Deq test
    Deq q=deq_new();
    Deq m=deq_new();

    char b[] = "Yes";
    char *i= b;
    char *k;

    printf("First list tests\n");
    
    // Test get from an empty deq
    deq_head_get(q);
    // Test rem from an empty deq
    deq_head_rem(q,i);
    // Test ith from an empty deq (Error thrown)
    // deq_tail_ith(q, 1);
    // Test put with Null data (Error thrown)
    // deq_tail_put(q, NULL);

    // Test head put from an empty deq
    deq_head_put(q, i);
    char *s=deq_str(q,0);
    printf("Head_put \"Yes\" in empty deq: %s\n",s);

    // Test rem from size-one deq
    i=deq_head_rem(q, i);
    printf("Head_rem \"Yes\" from size-one deq: %s\n",(char*) i);
    deq_head_put(q, i);

    // Test head put from size-one deq
    char w[] = "Nice";
    i=w;
    deq_head_put(q, i);
    free(s);
    s=deq_str(q,0);
    printf("Head_put \"Nice\" in size-one deq: %s\n",s);

    // Test rem from size-two deq
    k=deq_tail_rem(q, i);
    free(s);
    s=deq_str(q,0);
    printf("Tail_rem \"Nice\" from size-two deq: %s\n",(char*) k);
    printf("Tail_rem \"Nice\" list: %s\n",s);
    deq_tail_put(q, k);

    char *g;
    char d[] = "Man";
    i=d;
    // Test head put from size-two deq
    deq_head_put(q, i);
    g=deq_str(q,0);
    printf("tail_put from size-two deq: %s\n",g);
    // Test get from size-three deq
    k=deq_tail_get(q);
    free(s);
    s=deq_str(q,0);
    printf("Head_get from size-three deq: %s\n",(char*) k);
    printf("Head_get list: %s\n",s);
    deq_tail_put(q, k);

    
    // deq_tail_get(q);
    //Go through loop three times
    printf("Final first list print: ");
    free(g);
    g=deq_str(q,0);
    printf("%s\n",g);

    //Index tests
    i = deq_tail_ith(q, 1);
    printf("ITH_Tail %s from index 1\n", (char*) i);
    i = deq_head_ith(q, 0);
    printf("ITH_Head %s from index 0\n", (char*) i);
    printf("ITH no change to list: ");
    free(g);
    g=deq_str(q,0);
    printf("%s\n",g);

    free(g);
    g=deq_str(m,0);
    printf("%s\n",g);

    printf("\nSecond list tests\n");

    i=w; //Nice
    deq_tail_put(m, i);
    // Test tail put from size-one deq
    free(s);
    s=deq_str(m,0);
    printf("Tail_put \"Yes\" in empty deq: %s\n",s);
    k=deq_tail_get(m);
    // Test get from size-one deq
    printf("Tail_get from size-one deq: %s\n",(char*) k);
    deq_tail_put(m, k);
    i=b; //Yes
    // Test tail put from size-two deq
    deq_tail_put(m, i);
    free(s);
    s=deq_str(m,0);
    printf("Tail_put \"Nice\" in size-one deq: %s\n",s);
    // Test get from size-two deq
    k=deq_head_get(m);
    free(s);
    s=deq_str(m,0);
    printf("Head_get from size-two deq: %s\n",(char*) k);
    printf("Head_get list: %s\n",s);
    deq_tail_put(m, k);
    i=b; //Yes
    // Test tail put from size-two deq
    deq_tail_put(m, i);
    free(g);
    g=deq_str(m,0);
    printf("Tail_put from size-two deq: %s\n",g);
    // Test rem from size-three deq
    k=deq_tail_rem(m, i);
    free(s);
    s=deq_str(m,0);
    printf("Tail_rem \"Yes\" dupe from size-three deq: %s\n",(char*) k);
    printf("Tail_rem \"Yes\" dupe list: %s\n",s);
    //Restore final
    deq_tail_put(m, k);

    free(s);
    s=deq_str(m,0);
    printf("Final second list: %s\n",s);
    free(s);
    free(g);


    
    deq_del(q,0);
    deq_del(m,0);
    return 0;
}