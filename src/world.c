#include "world.h"
#include "vulkan_public.h"
#include <stdlib.h>

int worldInit(GameWorld* w, int width, int height){
  w->width = width;
  w->size = width * height;
  w->tiles = (Entity*)malloc(w->size * sizeof(Entity));

  Entity air = {
    .uv = 0,
    .fg = 15,
    .bg = 0,
    .collide = 0,
  };
  
  for(int y = 0; y < height; y++){
    for(int x = 0; x < w->width; x++){
      mapPutTile(w, air, x, y);
    }
  }
  w->actors = malloc(1 * sizeof(Entity));
  w->actors_count = 1;
  
  return 0;
}

Entity mapGetTile(GameWorld* map, int x, int y){
  Entity null_ent = {
    .uv = 370,
    .fg = HEX_COLOR_WHITE,
    .bg = HEX_COLOR_BLACK,
    .collide = 0,
    .pos = (Pos){ x, y},
  };
  if(x >= map->width) return null_ent;
  if(x < 0) return null_ent;
  if(y < 0) return null_ent;

  int offset = (y * map->width) + x;
  if(offset >= map->size) return null_ent;
  return map->tiles[offset];
}

int mapPutTile(GameWorld* map, Entity entity, int x, int y){

  if(x >= map->width) return 1;
  if(x < 0) return 1;
  if(y < 0) return 1;
  int offset = (y * map->width) + x;  
  if(offset >= map->size) return 1;
  
  entity.pos.x = x;
  entity.pos.y = y;
  map->tiles[offset] = entity;
  return 0;
}

int entityAdd(GameWorld* world, Entity src){
  src.inventory = malloc(sizeof(Entity)),
  world->actors[0] = src;
  return 0;
}

