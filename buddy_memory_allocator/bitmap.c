#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "bitmap.h"
#include "utils.h"

static const int bitsperbyte=8;

static int divup(int n, int d) {
  return (n+d-1)/d;
}

//Calculate size of bitmap for level in freelist
//@param size total size of free block allocated for the level
//@param e exponenent of 2^e for size of the block
static int mapsize(unsigned int size, int e) {
  int blocksize=e2size(e);
  int blocks=divup(size,blocksize);
  int buddies=divup(blocks,2);
  return divup(buddies,bitsperbyte);
}

//Get addr from bitmap
static int bitaddr(void *base, void *mem, int e) {
  int addr=buddyclr(base,mem,e)-base;
  int blocksize=e2size(e);
  return addr/blocksize/2;
}

//Create a new bitmap for a freeblock
//For params look at mapsize
extern BitMap bitmapnew(unsigned int size, int e) {
  int ms=mapsize(size,e);
  BitMap b=mmalloc(ms);
  if ((long)b==-1)
    return 0;
  memset(b,0,ms);
  return b;
}

// Set bit in bitmap 
extern void bitmapset(BitMap b, void *base, void *mem, int e) {
  int offset=bitaddr(base,mem,e);
  bitset(((unsigned char *)b)+offset/bitsperbyte,offset%bitsperbyte);
}

// Clear bit in bitmap
extern void bitmapclr(BitMap b, void *base, void *mem, int e) {
  int offset=bitaddr(base,mem,e);
  bitclr(((unsigned char *)b)+offset/bitsperbyte,offset%bitsperbyte);
}

// Test bit in bitmap
extern int bitmaptst(BitMap b, void *base, void *mem, int e) {
  int offset=bitaddr(base,mem,e);
  return bittst(((unsigned char *)b)+offset/bitsperbyte,offset%bitsperbyte);
}

// Print out a representation of the bitmap
extern void bitmapprint(BitMap b, unsigned int size, int e) {
  int ms=mapsize(size,e);
  int byte;
  char buf[50];
  for (byte=ms-1; byte>=0; byte--){
    int n = sprintf(buf, "%02x%s",((unsigned char *)b)[byte],(byte ? " " : "\n"));
    write(1, buf, n);
  }
    
    // printf("%02x%s",((unsigned char *)b)[byte],(byte ? " " : "\n"));
}
