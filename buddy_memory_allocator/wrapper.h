#ifndef WRAPPER_H
#define WRAPPER_H

#include <string.h>

extern void *malloc(size_t size);

extern void free(void *ptr);

extern void *realloc(void *ptr, size_t size);

#endif


