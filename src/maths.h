#ifndef MATHS_H
#define MATHS_H

typedef struct{
  float x;
  float y;
} vec2;

typedef struct{
  int x;
  int y;
} ivec2;

typedef struct{
  int x;
  int y;
  int z;
} ivec3;

static inline uint8_t pack_uint4(uint8_t a, uint8_t b) {
    return (a & 0x0F) | ((b & 0x0F) << 4);
}

#endif // MATHS_H
