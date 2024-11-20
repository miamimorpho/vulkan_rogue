#include "world.h"
#include "maths.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

struct GameObjectBufferImpl{
  size_t capacity;
  size_t count;
  GameObject* data;
};

GameObjectBuffer objectBufferCreate(size_t capacity){
  size_t* allocation = malloc(
    sizeof(size_t) * 2 +
    sizeof(GameObject) * capacity);
  if(!allocation) return NULL;

  allocation[0] = capacity;
  allocation[1] = 0;

  return (GameObjectBuffer)(allocation + 2);
}

void objectRemove(GameObjectBuffer objects, size_t index){
  size_t count = *bufferSize(objects);
  if(index >= count) return;
  
  memmove(&objects[index],
	  &objects[index +1],
	  (count - index - 1) * sizeof(GameObject) );

  (*bufferSize(objects))--;
}

int objectPush(GameObjectBuffer objects, GameObject src){
  size_t capacity = *bufferCapacity(objects);
  size_t count = *bufferSize(objects);
 
  if(count >= capacity) return 1;

  objects[count] = src;
  (*bufferSize(objects))++;
  
  return 0;
}

void objectBufferDestroy(GameObjectBuffer buffer){
  if(!buffer) return;
  free(BUFFER_METADATA(buffer));
}


MapChunk* mapChunkCreate(void){

  MapChunk* chunk = malloc(sizeof(MapChunk));
  chunk->terrain = objectBufferCreate(CHUNK_WIDTH * CHUNK_WIDTH);

  GameObject air = {
    OBJECT_TERRAIN,
    .unicode = 0,
    .atlas = DRAW_TEXTURE_INDEX,
    .fg = 15,
    .bg = 3,
    .data.terra = {
      .blocks_movement = 0,
      .blocks_sight = 0
    },
    .inventory = NULL
  };
  
  for(size_t i = 0; i < *bufferCapacity(chunk->terrain); i++){
    objectPush(chunk->terrain, air);
  }

  chunk->mobiles = objectBufferCreate(CHUNK_OBJECT_C);
 
  return chunk;
}

GameObject* mobileCreate(MapPosition dst){

  MapChunk* local_chunk = dst.chunk_ptr;
  
  GameObject proto_mob = {
    OBJECT_MOBILE,
    .unicode = 1,
    .atlas = ASCII_TEXTURE_INDEX,
    .fg = 15,
    .bg = 0,
    .data.mob = {
      .pos = dst
    },
    .inventory = objectBufferCreate(4)
  };
  GameObject proto_item = {
    OBJECT_ITEM,
    .unicode = 0,
    .atlas = ASCII_TEXTURE_INDEX,
    .fg = 15,
    .bg = 0,
    .data.item = {
      .id = 4
    },
    .inventory = NULL,
  };
  objectPush(proto_mob.inventory, proto_item);
  
  //printf("mobile create %u\n", proto.data.mob.pos.x);
  GameObjectBuffer mobiles = local_chunk->mobiles;
  if(objectPush(mobiles, proto_mob) != 0){
    printf("err\n");
    return NULL;
  }  

  return &mobiles[*bufferSize(mobiles) -1];
}

GameObject mapGetTerrain(MapPosition pos){
  MapChunk* chunk = pos.chunk_ptr;
  
  GameObject null_object = {
    OBJECT_TERRAIN,
    .unicode = 370,
    .atlas = DRAW_TEXTURE_INDEX,
    .fg = 3,
    .bg = 0,
    .data.terra = {
      .blocks_movement = 0,
      .blocks_sight = 1,
    }
  };

  uint32_t offset = (pos.y * CHUNK_WIDTH) + pos.x;  
  if(pos.x < 0 ||
     pos.x >= CHUNK_WIDTH ||
     offset >= *bufferSize(chunk->terrain)){
    return null_object;
  }

  return chunk->terrain[offset];
}

int mapSetTerrain(GameObject proto, MapPosition pos){
  if(proto.type != OBJECT_TERRAIN) return 1;

  MapChunk* chunk = pos.chunk_ptr;
  uint32_t offset = (pos.y * CHUNK_WIDTH) + pos.x;  
  if(pos.x < 0 ||
     pos.x >= CHUNK_WIDTH ||
     offset >= *bufferSize(chunk->terrain)){
    return 1;
  }

  chunk->terrain[offset] = proto;
  return 0;
}

/* Shadowcasting */

// Cardinal directions
enum {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};

// Row structure
typedef struct {
  int cardinal;
  int depth;
  Fraction start_slope;
  Fraction end_slope;
} Row;

// Transform coordinates based on quadrant
void transform_coords(int cardinal, int ox, int oy, int row, int col, int* out_x, int* out_y) {
    switch(cardinal) {
        case NORTH:
            *out_x = ox + col;
            *out_y = oy - row;
            break;
        case SOUTH:
            *out_x = ox + col;
            *out_y = oy + row;
            break;
        case EAST:
            *out_x = ox + row;
            *out_y = oy + col;
            break;
        case WEST:
            *out_x = ox - row;
            *out_y = oy + col;
            break;
    }
}

// Helper functions for rounding
int round_ties_up(double n) {
      return (int)floor(n + 0.5);
}

int round_ties_down(double n) {
   return (int)ceil(n - 0.5);
}

// Calculate slope as a fraction
Fraction slope(int row_depth, int col) {
    return fractionNew(2 * col - 1, 2 * row_depth);
}

bool is_symmetric(Row* row, int col)
{
  /* checks if slope (col/row) is 
   * between start and end slopes */
    int depth_mul_start = row->depth * row->start_slope.num;
  int col_div_start = col * row->start_slope.den;
  
  int depth_mul_end = row->depth * row->end_slope.num;
  int col_div_end = col * row->end_slope.den;
  
  return (col_div_start >= depth_mul_start &&
	  col_div_end <= depth_mul_end);
}

bool is_blocking(MapPosition pos) {
  GameObject t = mapGetTerrain(pos);
  if(t.data.terra.blocks_sight == 1)
    return true;

  return false;
}

void mark_visible(GameObjectBuffer to_draw, MapPosition pos) {
  GameObject terrain = mapGetTerrain(pos);
  terrain.type = OBJECT_MOBILE;
  terrain.data.mob.pos = pos;
  objectPush(to_draw, terrain);
}

void shadowcast_scan_row(GameObjectBuffer to_draw, MapPosition camera, Row current_row){
  
  // allows early termination on invalid angles
  if (fractionCompare(current_row.end_slope, current_row.start_slope)) {
    return;
  }

  // int * fraction
  int min_col = round_ties_up( (double)
    (current_row.depth * current_row.start_slope.num)
    / (double)current_row.start_slope.den);
  int max_col = round_ties_down( (double)
    (current_row.depth * current_row.end_slope.num)
    / (double)current_row.end_slope.den);
  
  bool prev_was_wall = false;
 
  for(int col = min_col; col <= max_col; col++) {    
    int target_x, target_y;
    transform_coords(current_row.cardinal, camera.x, camera.y, 
		     current_row.depth, col, &target_x, &target_y);

    MapPosition current_pos = 
      { target_x, target_y, camera.chunk_ptr };
    bool is_wall = is_blocking(current_pos);
    
    if(is_wall || is_symmetric(&current_row, col)) {
      if(relativeDistance(current_row.depth, col) < 12.5 * 12.5)
	mark_visible(to_draw, current_pos);	  
    }
    
    if(prev_was_wall && !is_wall) {
      current_row.start_slope = slope(current_row.depth, col);
    }
    
    if(!prev_was_wall && is_wall) {
      Row next_row = {
	.cardinal = current_row.cardinal,
	.depth = current_row.depth + 1,
	.start_slope = current_row.start_slope,
	.end_slope = slope(current_row.depth, col)
      };
      shadowcast_scan_row(to_draw, camera, next_row);
    }
    
    prev_was_wall = is_wall;
  }// end of row scanning
  
  // if floor at last tile, create new tile
  if(!prev_was_wall) {
    Row next_row = {
      .cardinal = current_row.cardinal,
      .depth = current_row.depth + 1,
      .start_slope = current_row.start_slope,
      .end_slope = current_row.end_slope
    };
    shadowcast_scan_row(to_draw, camera, next_row);
  }
  
}

GameObjectBuffer shadowcast_fov(MapPosition camera){

  GameObjectBuffer to_draw = objectBufferCreate(3000);

  mark_visible(to_draw, camera);
  for(int cardinal = 0; cardinal < 4; cardinal++) {
    
    Row first_row = {
      .cardinal = cardinal,
      .depth = 1,
      .start_slope = fractionNew(-1, 1),
      .end_slope = fractionNew(1, 1)
    };
    shadowcast_scan_row(to_draw, camera, first_row);
    
  }
  return to_draw;
}

int mapChunkDraw(Gfx gfx, MapPosition camera){

  gfxClear(gfx);
  
  int x_offset = (ASCII_SCREEN_WIDTH / 2);
  int y_offset = (ASCII_SCREEN_HEIGHT / 2);

  GameObjectBuffer to_draw = shadowcast_fov(camera);
  size_t draw_count = *bufferSize(to_draw);

  for(unsigned int i = 0; i < draw_count; i++){
    GameObject tile = to_draw[i];
    int tile_x = tile.data.mob.pos.x - camera.x + x_offset;
    int tile_y = tile.data.mob.pos.y - camera.y + y_offset;
    gfxAddCh(gfx, tile_x, tile_y, tile.unicode, tile.atlas, tile.fg, tile.bg);
  }
  objectBufferDestroy(to_draw);
 
  // render list of entities (actors)
  // TODO: line of sight
  GameObjectBuffer mobiles = camera.chunk_ptr->mobiles;
  for(size_t i = 0; i < *bufferSize(mobiles); i++){
    GameObject actor = mobiles[i];
    gfxAddCh(gfx, x_offset, y_offset,
	     actor.unicode, actor.atlas,
	     actor.fg, actor.bg);
  }
  return 0;
}
