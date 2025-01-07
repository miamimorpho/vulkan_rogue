#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mystdlib.h"

/* Heap based stack allocator */
static const ptrdiff_t ARCH_ALIGNMENT = 2 * sizeof(void*);

#define UINT32_DEADBEEF 0xDEADBEEF

struct MemArena{
    char* beg;
    char* offset;
    char* end;
};

void* memArenaSuballoc(MemArena *a, size_t size){
    assert(a);
  
    ptrdiff_t padding = -(uintptr_t)a->offset & (ARCH_ALIGNMENT - 1);
    // derived from 'extra = addr % align' assuming align is always a power of 2
    size_t available = a->end - a->offset - padding;
    if(available < size){
        fprintf(stderr,"ERR memArenaSuballoc: tried allocating %zu / %zu\n", size, available);
        return NULL;
    }

//   void *p = a->offset + padding;
void *p = (char*)((uintptr_t)a->offset + padding);
   a->offset += padding + size;
   return memset(p, 0, size);
}

void memArenaPop(MemArena* a, void* allocation){
    char* p_offset = (char*)allocation;
    assert(a);
    if(p_offset < a->end && p_offset > a->beg){
        ptrdiff_t size = a->offset - p_offset;
        a->offset = p_offset;
        memset(a->offset, 0, size);
    }
}

MemArena* memArenaMalloc(size_t size) {
    // Single malloc for both data and struct
    char* mem = malloc(size + sizeof(MemArena));
    if (mem == NULL) {
        fprintf(stderr, "not enough ram to allocate MemArena\n");
        return NULL;
    }
    
    // Initialize the arena struct at the end
    MemArena* arena = (MemArena*)(mem + size);
    arena->beg = mem;
    arena->offset = mem;
    arena->end = mem + size;
    
    return arena;
}


void memArenaDestroy(MemArena *a){
    assert(a);
    free(a->beg);
}

struct AllocatorInterface memArenaCreate(size_t size){
    return (struct AllocatorInterface){
        .mallocFn = memArenaSuballoc,
        .freeFn = memArenaPop,
        .ctx = memArenaMalloc(size),
        };
}

struct MemSlice{
    uint32_t sentinal;
    size_t size;
    char data[];
};

static inline struct MemSlice* memSliceInfo(void* data_ptr){
   struct MemSlice* s = container_of(data_ptr, struct MemSlice, data);
   if(s->sentinal == UINT32_DEADBEEF) return s;
   return NULL;
}
size_t memSliceSize(void* data_ptr){
    return memSliceInfo(data_ptr)->size;
}

void* memSliceCreate(size_t nmemb, size_t stride, struct AllocatorInterface allocator){
    if(stride && nmemb > SIZE_MAX / stride){
        fprintf(stderr, "nmemb %zu stride %zu\n", nmemb, stride);
        fprintf(stderr, "memSliceCreate: integer overflow\n");
        return NULL;
    }
  
    size_t total_size = sizeof(struct MemSlice) + nmemb * stride;
    struct MemSlice* s =  allocator.mallocFn(allocator.ctx, total_size);
    s->sentinal = UINT32_DEADBEEF;
    s->size = nmemb * stride;
    return s->data;
}

void memSliceDestroy(void* s_ptr, struct AllocatorInterface allocator){
    struct MemSlice* s = memSliceInfo(s_ptr);
    allocator.freeFn(allocator.ctx, s);
}

/* Bitmaps */

struct Bitmap{
  uint32_t width;
  uint32_t height;
  char data[];
};

#define BITMASK_AT(offset) (1 << (7 - (offset)))

static inline uint32_t roundBitUp(uint32_t bit_index){
  // add 7, clear last three bits, rounding down
  return (bit_index + 7) & ~7; 
}

static inline uint8_t getBitInByte(uint8_t offset, uint8_t byte){
  return byte & BITMASK_AT(offset) ? 1 : 0;
}

static inline size_t bitmapDataSize(uint32_t width, uint32_t height){
  return (roundBitUp(width) * roundBitUp(height)) / 8;
}

void bitmapFill(struct Bitmap* bmp, uint8_t val){
  if(bmp) {
    size_t size = bitmapDataSize(bmp->width, bmp->height);
    if(val) val = 255;
    memset(bmp->data, val, size);
  }
}

struct Bitmap* bitmapCreate(uint32_t width, uint32_t height, AllocatorInterface allocator){

  size_t total_size = sizeof(Bitmap) + bitmapDataSize(width, height);;

  Bitmap* bmp = allocator.mallocFn(allocator.ctx, total_size);
  bmp->width = width;
  bmp->height = height;
  bitmapFill(bmp, 0);

  return bmp;
}

void bitmapDestroy(Bitmap* bmp, AllocatorInterface allocator){
  allocator.freeFn(allocator.ctx, bmp);
}

uint8_t bitmapGetPx(struct Bitmap* bmp, int32_t x, int32_t y){
  if( x < 0 || x >= (int)bmp->width ||
      y < 0 || y >= (int)bmp->height){
    return 1;
  }
  int bit_index = y * bmp->width + x;
  int offset = bit_index % 8;
  uint8_t byte = bmp->data[bit_index / 8];
  return getBitInByte(offset, byte);
}

uint8_t bitmapSetPx(struct Bitmap* bmp, int32_t x, int32_t y, uint8_t val){
  if( x < 0 || x >= (int)bmp->width ||
      y < 0 || y >= (int)bmp->height){
    return 1;
  }
  int bit_index = y * bmp->width + x;
  int offset = bit_index % 8;
  char* dst = &bmp->data[bit_index / 8];
  
  if(val){
    *dst |= BITMASK_AT(offset);
  }else{
    *dst &= ~BITMASK_AT(offset);
  }
  return 0;
}
