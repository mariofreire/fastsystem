#ifndef _MEMORY_H_
#define _MEMORY_H_

static size_t align_up(size_t value, size_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

void *realloc(void *ptr, size_t new_size);
void* calloc(size_t num, size_t size);
void *malloc(size_t size);
void free(void *ptr);

void map_page(void *physaddr, void *virtualaddr, unsigned int flags);
void unmap_page(void *virtualaddr);
void *palloc(void);
void pfree(void *physaddr);
void flushpage(unsigned long addr);

#endif // _MEMORY_H_
