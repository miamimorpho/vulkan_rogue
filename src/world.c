#include "world.h"
#include <stdlib.h>

int worldInit(gameWorld_t* w, int size){
  w->width = size;
  w->size = size * size;
  w->tiles = (entity_t*)malloc(w->size * sizeof(entity_t));
  entity_t terrain = {
    .ch = '.'
  };
  for(int x = 0; x < w->width; x++){
    for(int y = 0; y < w->width; y++){
      mapPutTile(w, terrain, x, y);
    }
  }
  w->actors = malloc(1 * sizeof(entity_t));
  w->actors_count = 1;
  
  return 0;
}

entity_t mapGetTile(gameWorld_t* map, int x, int y){
  entity_t null_ent = {
    .ch = '.',
    .pos = (pos_t){ x, y},
  };
  if(((y * map->width) + x) > map->size) return null_ent;
  return map->tiles[(y * map->width) + x ];
}

int mapPutTile(gameWorld_t* map, entity_t entity, int x, int y){
  if(((y * map->width) + x) > map->size) return 1;
  entity.pos.x = x;
  entity.pos.y = y;
  map->tiles[(y * map->width) +x ] = entity;
  return 0;
}

int entityMove(entity_t* e, int x, int y){
  e->pos.x += x;
  e->pos.y += y;
  
  return 0;
}

int entityAdd(gameWorld_t* world, entity_t src){
  world->actors[0] = src;
  return 0;
}
