#include "world.h"
#include <stdlib.h>

int worldInit(GameWorld* w, int size){
  w->width = size;
  w->size = size * size;
  w->tiles = (Entity*)malloc(w->size * sizeof(Entity));
  Entity terrain = {
    .ch = '.'
  };
  for(int x = 0; x < w->width; x++){
    for(int y = 0; y < w->width; y++){
      mapPutTile(w, terrain, x, y);
    }
  }
  w->actors = malloc(1 * sizeof(Entity));
  w->actors_count = 1;
  
  return 0;
}

Entity mapGetTile(GameWorld* map, int x, int y){
  Entity null_ent = {
    .ch = '.',
    .pos = (Pos){ x, y},
  };
  int offset = (y * map->width) + x;
  if(offset > map->size) return null_ent;
  return map->tiles[offset];
}

int mapPutTile(GameWorld* map, Entity entity, int x, int y){
  int offset = (y * map->width) + x;
  if(offset > map->size) return 1;
  entity.pos.x = x;
  entity.pos.y = y;
  map->tiles[offset] = entity;
  return 0;
}

int entityMove_f(Entity* e, int x, int y){
  e->pos.x += x;
  e->pos.y += y;
  
  return 0;
}

int entityAdd(GameWorld* world, Entity src){
  world->actors[0] = src;
  return 0;
}
