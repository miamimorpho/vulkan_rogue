#ifndef WORLD_H
#define WORLD_H
#include "vulkan_public.h"
#include <stdint.h>

typedef struct{
  int x;
  int y;
} Pos;

typedef struct Entity Entity;
struct Entity{
  int is_init;
  Pos pos; // make xy
  int uv;
  uint32_t fg;
  uint32_t bg;
  int collide;
  Entity* inventory;
  int inventory_c;
};

typedef struct{
  Entity* tiles;
  int width;
  int size;
  Entity* props;
  Entity* actors;
  unsigned int actors_count;
} GameWorld;


int mapPutTile(GameWorld*, Entity, int, int);
Entity mapGetTile(GameWorld, int x, int y);
int worldInit(GameWorld*, int, int);
Entity* entityInit(GameWorld*, unsigned int);
int worldDraw(Gfx, GameWorld, Entity);

#endif // WORLD_H
