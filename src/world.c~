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
  int actor_c = 8;
  w->actors = malloc(actor_c * sizeof(Entity));
  w->actors_count = actor_c;
  for(int i = 0; i < actor_c; i++){
    w->actors[i].is_init = 0;
  }
  
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

Entity* entityInit(GameWorld* world, uint entity_index){

  if(entity_index > world->actors_count){
    return NULL;
  }
  
  Entity* e = &world->actors[entity_index];
  e->is_init = 1;
  e->pos = (Pos){0, 0};
  e->uv = 1024;
  e->fg = 15;
  e->bg = 0;
  e->collide = 1;
  e->inventory = NULL;
  e->inventory_c = 0;
    
  return e;
}

Entity* entityGet(GameWorld* world, uint entity_index){
  if(entity_index > world->actors_count){
    return NULL;
  }
  return &world->actors[entity_index];
}
