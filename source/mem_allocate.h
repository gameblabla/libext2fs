#ifndef _MEM_ALLOCATE_H
#define _MEM_ALLOCATE_H

#include <malloc.h>

extern void* mem_alloc (size_t size);

extern void* mem_calloc (size_t count, size_t size) ;

extern void* mem_realloc (void *p, size_t size);

extern void* mem_align (size_t a, size_t size);
extern void mem_free (void* mem);

#endif /* _MEM_ALLOCATE_H */
