#ifndef MATHS_H
#define MATHS_H

#include <math.h>
#include <stdbool.h>

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

typedef struct {
  int num;
  int den;
} Fraction;

// Helper functions for fractions
static inline Fraction fractionNew(int num, int den) {
  Fraction f = {num, den};
  return f;
}

static inline bool fractionCompare(Fraction small, Fraction large){
  return(small.num * large.den <= large.num * small.den);
}

// Maths
static inline int iMax(int a, int b) {
    return (a > b) ? a : b;
}

static inline int iMin(int a, int b) {
    return (a < b) ? a : b;
}

// Vector Function
static inline int relativeDistance(int dx, int dy){
  return (dx * dx) + (dy * dy);
}

// Packer Functions
static inline uint8_t pack_uint4(uint8_t a, uint8_t b) {
  return (a & 0x0F) | ((b & 0x0F) << 4);
}

#endif // MATHS_H
