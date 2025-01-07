#ifndef MYSTDLIB
#define MYSTDLIB

#include <stdint.h>
#include <stddef.h>

#define KB (1024ULL)
#define MB (1024ULL * KB)
#define GB (1024ULL * MB)
#define TB (1024ULL * GB)

#define container_of(ptr, type, member) ((type*)((char *)(ptr) - offsetof(type, member)))

typedef struct MemArena MemArena;
typedef struct AllocatorInterface AllocatorInterface;
struct AllocatorInterface{
    void* (*mallocFn)(MemArena*, size_t);
    void  (*freeFn)(MemArena*, void*);
    void  *ctx;
};
AllocatorInterface memArenaCreate(size_t size);
void memArenaDestroy(MemArena*);

void* memSliceCreate(size_t, size_t, AllocatorInterface);
size_t memSliceSize(void*);
void memSliceDestroy(void*, AllocatorInterface);

typedef struct Bitmap Bitmap;
Bitmap* bitmapCreate(uint32_t, uint32_t, AllocatorInterface);
void bitmapDestroy(Bitmap*, AllocatorInterface);
void bitmapFill(Bitmap*, uint8_t);
uint8_t bitmapGetPx(Bitmap*, int32_t, int32_t);
uint8_t bitmapSetPx(Bitmap*, int32_t, int32_t, uint8_t);

// TODO: hashmap implementation

#endif
