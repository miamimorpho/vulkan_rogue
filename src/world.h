#ifndef WORLD_H
#define WORLD_H
#include "vulkan_public.h"
#include <stdint.h>

typedef struct{
  int x;
  int y;
} Pos;

typedef struct Entity Entity;

typedef struct{
  size_t capacity;
  size_t count;
  Entity* data;
} EntityArray;

struct Entity{
  int is_init;
  int x;
  int y;
  int uv;
  uint32_t fg;
  uint32_t bg;
  int collide;
  int blocks_sight;
  EntityArray inventory;
};

typedef struct{
  Entity* tiles;
  int width;
  int size;
} GameWorldTerrain;

typedef struct{
  GameWorldTerrain terrain;
  Entity* props;
  Entity* actors;
  unsigned int actors_count;
} GameWorld;

int mapPutTile(GameWorldTerrain*, Entity, int, int);
Entity mapGetTile(GameWorldTerrain, int x, int y);
int worldInit(GameWorld*, int, int);
Entity* entityInit(GameWorld*, unsigned int);
int worldDraw(Gfx, GameWorld, Entity);

#endif // WORLD_H
