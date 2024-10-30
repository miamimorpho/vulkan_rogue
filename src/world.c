#include "world.h"
#include "vulkan_public.h"
#include <stdlib.h>

int worldDraw(GameWorld world, Entity camera){
 
  int x_offset = (ASCII_SCREEN_WIDTH / 2);
  int y_offset = (ASCII_SCREEN_HEIGHT / 2);
  
  for(int y = 0; y < ASCII_SCREEN_HEIGHT; y++){
    for(int x = 0; x < ASCII_SCREEN_WIDTH; x++){
      Entity tile = mapGetTile(world,
			       camera.pos.x - x_offset +x,
			       camera.pos.y - y_offset +y);
      gfxAddCh(x, y,
	       tile.uv, DRAW_TEXTURE_INDEX,
	       tile.fg, tile.bg);
    }
  }
  
  // render list of entities (actors)
  for(unsigned int i = 0; i < world.actors_count; i++){
    Entity actor = world.actors[i];
    if(actor.is_init == 1){
      gfxAddCh(x_offset, y_offset,
	       actor.uv, DRAW_TEXTURE_INDEX,
	       actor.fg, actor.bg);
    }
  }
  return 0;
}

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

Entity mapGetTile(GameWorld arr, int x, int y){
  Entity null_ent = {
    .uv = 370,
    .fg = 1,
    .bg = 0,
    .collide = 0,
    .pos = (Pos){ x, y},
  };
  if(x >= arr.width) return null_ent;
  if(x < 0) return null_ent;
  if(y < 0) return null_ent;

  int offset = (y * arr.width) + x;
  if(offset >= arr.size) return null_ent;
  return arr.tiles[offset];
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

Entity* entityInit(GameWorld* world, unsigned int entity_index){

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

Entity* entityGet(GameWorld* world, unsigned int entity_index){
  if(entity_index > world->actors_count){
    return NULL;
  }
  return &world->actors[entity_index];
}
