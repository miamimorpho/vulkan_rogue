#include "world.h"
#include "maths.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct GameObjectBufferInfo*
bufferInfo(GameObjectBuffer buffer){
  return ( ( (struct GameObjectBufferInfo*)(buffer) ) - 1 );
}

GameObjectBuffer objectBufferCreate(size_t capacity){
  struct GameObjectBufferInfo* allocation = malloc(
    sizeof(struct GameObjectBufferInfo) +
    sizeof(GameObject) * capacity);
  if(!allocation) return NULL;

  allocation->count = 0;
  allocation->capacity = capacity;

  return (GameObjectBuffer)(allocation +1);
}

void objectRemove(GameObjectBuffer objects, size_t index){
  
  struct GameObjectBufferInfo* info = bufferInfo(objects);
  if(index >= info->count) return;
  
  memmove(&objects[index],
	  &objects[index +1],
	  (info->count - index - 1) * sizeof(GameObject) );

  info->count--;
}

int objectPush(GameObjectBuffer objects, GameObject src){

  struct GameObjectBufferInfo* info = bufferInfo(objects);
 
  if(info->count >= info->capacity) return 1;

  objects[info->count] = src;
  info->count++;
  
  return 0;
}

void objectBufferDestroy(GameObjectBuffer buffer){
  if(!buffer) return;
  free(bufferInfo(buffer));
}

MapChunk* mapChunkCreate(void){

  const int size = CHUNK_WIDTH * CHUNK_WIDTH;
  
  MapChunk* chunk = malloc(sizeof(MapChunk));
 
  GameObject air = {
    OBJECT_ITEM,
    .unicode = 0,
    .atlas = 2,//DRAW_TEXTURE_INDEX
    .fg = 15,
    .bg = 3,
    .type.item = {
      .id = 0,
    },
    .inventory = NULL
  };

  chunk->blocks_sight_bmp = bitMapCreate(CHUNK_WIDTH, CHUNK_WIDTH);
  chunk->blocks_movement_bmp = bitMapCreate(CHUNK_WIDTH, CHUNK_WIDTH);

  chunk->terrain = objectBufferCreate(size);
  for(size_t i = 0; i < size; i++){
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
    .atlas = 1,//ASCII_TEXTURE_INDEX,
    .fg = 15,
    .bg = 0,
    .type.mob = {
      .pos = dst
    },
    .inventory = objectBufferCreate(4)
  };
  GameObject proto_item = {
    OBJECT_ITEM,
    .unicode = 0,
    .atlas = 1,//ASCII_TEXTURE_INDEX,
    .fg = 15,
    .bg = 0,
    .type.item = {
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

  return &mobiles[bufferInfo(mobiles)->count -1];
}

GameObject mapGetTerrain(MapPosition pos){
  MapChunk* chunk = pos.chunk_ptr;
  
  GameObject null_object = {
    OBJECT_TERRAIN,
    .unicode = 370,
    .atlas = 2,//DRAW_TEXTURE_INDEX,
    .fg = 3,
    .bg = 0,
    .type.terra = {
      .blocks_movement = 0,
      .blocks_sight = 1,
    }
  };

  uint32_t offset = (pos.y * CHUNK_WIDTH) + pos.x;  
  if(pos.x < 0 ||
     pos.x >= CHUNK_WIDTH ||
     offset >= bufferInfo(chunk->terrain)->count){
    return null_object;
  }

  return chunk->terrain[offset];
}

uint8_t terraBlocksMove(MapPosition pos){
  MapChunk* chunk = pos.chunk_ptr;
  return bitMapGetPx(chunk->blocks_movement_bmp, pos.x, pos.y);
}

uint8_t terraBlocksSight(MapPosition pos) {
  MapChunk* chunk = pos.chunk_ptr;
  return bitMapGetPx(chunk->blocks_sight_bmp, pos.x, pos.y);
}

int terraSet(GameObject proto, MapPosition pos){
  if(proto.type_enum != OBJECT_TERRAIN) return 1;

  MapChunk* chunk = pos.chunk_ptr;
  uint32_t offset = (pos.y * CHUNK_WIDTH) + pos.x;  
  if(pos.x < 0 ||
     pos.x >= CHUNK_WIDTH ||
     offset >= bufferInfo(chunk->terrain)->count){
    return 1;
  }

  if(proto.type.terra.blocks_sight == 1){
    BitMap* blocks_sight = chunk->blocks_sight_bmp;
    bitMapSetPx(blocks_sight, pos.x, pos.y, 1);
  }
  if(proto.type.terra.blocks_movement == 1){
    BitMap* blocks_movement = chunk->blocks_movement_bmp;
    bitMapSetPx(blocks_movement, pos.x, pos.y, 1);
  }

  proto.type_enum = OBJECT_ITEM;
  proto.type.item.id = 0;
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

void shadowcastMarkVisible(BitMap* shadow_mask, MapPosition pos) {
  bitMapSetPx(shadow_mask, pos.x, pos.y, 0);
}

void shadowcastScanRow(BitMap* dst_mask, MapPosition camera, Row current_row){
  
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
    uint8_t is_wall = terraBlocksSight(current_pos);
    
    if(is_wall || is_symmetric(&current_row, col)) {
      if(relativeDistance(current_row.depth, col) < 12.5 * 12.5)
	shadowcastMarkVisible(dst_mask, current_pos);	  
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
      shadowcastScanRow(dst_mask, camera, next_row);
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
    shadowcastScanRow(dst_mask, camera, next_row);
  }
  
}

BitMap* shadowcastFOV(MapPosition camera){

  BitMap* shadow_mask = bitMapCreate(128, 128);
  bitMapFill(shadow_mask, 1);
  
  shadowcastMarkVisible(shadow_mask, camera);
  for(int cardinal = 0; cardinal < 4; cardinal++) {
    
    Row first_row = {
      .cardinal = cardinal,
      .depth = 1,
      .start_slope = fractionNew(-1, 1),
      .end_slope = fractionNew(1, 1)
    };
    shadowcastScanRow(shadow_mask, camera, first_row);
    
  }
  return shadow_mask;
}

int mapChunkDraw(Gfx gfx, MapPosition camera){

  gfxClear(gfx);

  int screen_width = 0;
  int screen_height = 0;
  gfxGetScreenDimensions(gfx, &screen_width, &screen_height);
  
  int x_offset = camera.x - (screen_width / 2);
  int y_offset = camera.y - (screen_height / 2);

  BitMap* shadow_mask = shadowcastFOV(camera);

  GameObjectBuffer terrain = camera.chunk_ptr->terrain;
  struct GameObjectBufferInfo info = *bufferInfo(terrain);
  //printf("%zu %zu %zu\n", info.capacity, info.count, draw_count);
  for(unsigned int i = 0; i < info.count; i++){
    int shadow_x = i % CHUNK_WIDTH;
    int shadow_y = i / CHUNK_WIDTH;
    if(bitMapGetPx(shadow_mask, shadow_x, shadow_y) == 0){
      GameObject tile = terrain[i];
      int tile_x = shadow_x - x_offset;
      int tile_y = shadow_y - y_offset;
      gfxAddCh(gfx, tile_x, tile_y, tile.unicode, tile.atlas, tile.fg, tile.bg);
    }
    
  }

  GameObjectBuffer mobiles = camera.chunk_ptr->mobiles;
  for(size_t i = 0; i < bufferInfo(mobiles)->count; i++){
    
    GameObject actor = mobiles[i];
    MapPosition pos = actor.type.mob.pos;
    if(bitMapGetPx(shadow_mask, actor.type.mob.pos.x, actor.type.mob.pos.y) == 0){
      gfxAddCh(gfx, pos.x - x_offset, pos.y - y_offset,
	     actor.unicode, actor.atlas,
	     actor.fg, actor.bg);
    }
  }

  bitMapDestroy(shadow_mask);
  
  return 0;
}
