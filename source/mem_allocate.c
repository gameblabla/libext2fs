#include <malloc.h>

void* mem_alloc (size_t size) {
    return malloc(size);
}
/*
void* mem_calloc (size_t count, size_t size) {
    return calloc(count, size);
}
*/
void* mem_realloc (void *p, size_t size) {
    return realloc(p, size);
}

void* mem_align (size_t a, size_t size) {
    #ifdef __wii__
    return memalign(a, size);
    #else
    return malloc(size);
    #endif
}
/*
void mem_free (void* mem) {
    free(mem);
}
*/
