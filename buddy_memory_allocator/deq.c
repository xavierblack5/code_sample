#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

// indices and size of array of node pointers
typedef enum {Head,Tail,Ends} End;

typedef struct Node {
  struct Node *np[Ends];		// next/prev neighbors
  Data data;
} *Node;

typedef struct {
  Node ht[Ends];			// head/tail nodes
  int len;
} *Rep;

static Rep rep(Deq q) {
  if (!q) ERROR("zero pointer");
  return (Rep)q;
}

static void put(Rep r, End e, Data d) {
  //Check if data is null
  if(!d) ERROR("Cannot add null data to deq");
  //Initialize and allocate Node
  Node n=(Node)malloc(sizeof(*n));
  if(!n) ERROR("malloc() failed");
  n->np[e]=0;
  n->np[e^1]=0;
  n->data=d;
  //Check if deq is empty, if true then add data as both head and tail
  if(r->len ==  0) {
    r->ht[e]=n;
    r->ht[e ^ 1]=n;
  }
  else {
    Node p=r->ht[e];
    r->ht[e]=n;
    n->np[e ^ 1]=p;
    p->np[e]=n;
  }
  //Increment len
  r->len++;
}
static Data ith(Rep r, End e, int i) {

  if((i+1) > r->len) ERROR("Index out of bounds");
  Node n=r->ht[e]; 
  for (int t = 0; t < i; t++) {
    n=n->np[e ^ 1];
  }
  return n->data; 
}
static Data get(Rep r, End e) { 
  //Check if there is anything in the list
  if(r->len == 0) {
    WARN("Cannot get from an empty deq");
    return 0;
  }
  //Grab desired end node and then set new end node
  Node n=r->ht[e];
  if(r->len == 1) {
    r->ht[e]=0;
    r->ht[e ^ 1]=0;
  }
  else {
    r->ht[e]=n->np[e ^ 1]; 
    r->ht[e]->np[e]=0;
  }
  Data d=n->data;
  //Decrement len
  r->len--;
  free(n);
  //possible problems as memory was freed
  return d;
}
static Data rem(Rep r, End e, Data d) { 
  for (Node n=r->ht[e]; n; n=n->np[e ^ 1]) {
    if(n->data == d) {
      if(r->len == 1) {
        r->ht[e]=0;
        r->ht[e ^ 1]=0;
      }
      else {
        if(n == r->ht[e]) r->ht[e]=n->np[e ^ 1];
        if(n == r->ht[e ^ 1]) r->ht[e ^ 1]=n->np[e];
        //Pointers need to be checked for null before doing anything to them
        Node p=n->np[e];
        Node a=n->np[e ^ 1];
        if(p && a) {
          p->np[e ^ 1]=a;
          a->np[e]=p;
        }
        else if(p){
          p->np[e ^ 1]=0;
        }
        else a->np[e]=0;
      }
      Data b=n->data;
      free(n);
      r->len--;
      return b;
    }
  }
  if(r->len == 0) WARN("Cannot remove pointer: %p, Not found in the deq", d);
  return 0; 
}

extern Deq deq_new() {
  Rep r=(Rep)malloc(sizeof(*r));
  if (!r) ERROR("malloc() failed");
  r->ht[Head]=0;
  r->ht[Tail]=0;
  r->len=0;
  return r;
}

extern int deq_len(Deq q) { return rep(q)->len; }

extern void deq_head_put(Deq q, Data d) {        put(rep(q),Head,d); }
extern Data deq_head_get(Deq q)         { return get(rep(q),Head); }
extern Data deq_head_ith(Deq q, int i)  { return ith(rep(q),Head,i); }
extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q),Head,d); }

extern void deq_tail_put(Deq q, Data d) {        put(rep(q),Tail,d); }
extern Data deq_tail_get(Deq q)         { return get(rep(q),Tail); }
extern Data deq_tail_ith(Deq q, int i)  { return ith(rep(q),Tail,i); }
extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q),Tail,d); }

extern void deq_map(Deq q, DeqMapF f) {
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail])
    f(n->data);
}

extern void deq_del(Deq q, DeqMapF f) {
  if (f) deq_map(q,f);
  Node curr=rep(q)->ht[Head];
  while (curr) {
    Node next=curr->np[Tail];
    free(curr);
    curr=next;
  }
  free(q);
}

extern Str deq_str(Deq q, DeqStrF f) {
  char *s=strdup("");
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail]) {
    char *d=f ? f(n->data) : n->data;
    char *t; asprintf(&t,"%s%s%s",s,(*s ? " " : ""),d);
    free(s); s=t;
    if (f) free(d);
  }
  return s;
}
