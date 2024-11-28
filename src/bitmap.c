#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"

#define BITMASK_AT(offset) (1 << (7 - (offset)))

static inline uint32_t
roundBitUp(uint32_t bit_index){
  // add 7, clear last three bits, rounding down
  return (bit_index + 7) & ~7; 
}

static inline uint8_t
getBitInByte(uint8_t offset, uint8_t byte){
  return byte & BITMASK_AT(offset) ? 1 : 0;
}

BitMap* bitMapCreate(uint32_t width, uint32_t height){

  width = roundBitUp(width);
  height = roundBitUp(height); 
  size_t size = (width * height) / 8;
  
  BitMap* bmp = malloc(sizeof(BitMap) + size);
  if(bmp == NULL) return NULL;
  
  memset(bmp->data, 0, size);
  bmp->width = width;
  bmp->height = height;
  
  return bmp;
}

void bitMapDestroy(BitMap* bmp){
  if(bmp) free(bmp);
}

void bitMapFill(BitMap* bmp, uint8_t val) {
  if(bmp) {
    size_t size = (bmp->width * bmp->height) / 8;
    if(val) val = 255;
    memset(bmp->data, val, size);
  }
}

uint8_t bitMapGetPx(BitMap* bmp, int32_t x, int32_t y){
  if( x < 0 || x >= (int)bmp->width ||
      y < 0 || y >= (int)bmp->height){
    return 1;
  }
  int bit_index = y * bmp->width + x;
  int offset = bit_index % 8;
  uint8_t byte = bmp->data[bit_index / 8];
  return getBitInByte(offset, byte);
}

uint8_t bitMapSetPx(BitMap* bmp, int32_t x, int32_t y, uint8_t val){
  if( x < 0 || x >= (int)bmp->width ||
      y < 0 || y >= (int)bmp->height){
    return 1;
  }
  int bit_index = y * bmp->width + x;
  int offset = bit_index % 8;
  uint8_t* dst = &bmp->data[bit_index / 8];
  
  if(val){
    *dst |= BITMASK_AT(offset);
  }else{
    *dst &= ~BITMASK_AT(offset);
  }
  return 0;
}
