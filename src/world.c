#include "world.h"
#include "maths.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mystdlib.h"

GameObjectBuffer createObjectBuffer(size_t nmemb)
{
  return myBufferMalloc(nmemb, sizeof(GameObject));
}

GameObject* objectPush(GameObjectBuffer dst, GameObject* src){
  return myBufferPush(dst, src, sizeof(GameObject));
}

int objectPop(GameObjectBuffer dst){
  return myBufferPop(dst);
}

void objectBufferDestroy(GameObjectBuffer target){
    myBufferFree(target);
}

MapChunkBuffer createMapChunkBuffer(size_t nmemb){
   return myBufferMalloc(nmemb, sizeof(MapChunk));
}

WorldArena createWorldArena(void){
  WorldArena arena;
  arena.mobiles = createObjectBuffer(10); 
  arena.map_chunks = createMapChunkBuffer(10);
  MapChunk* zeroeth_chunk = newMapChunk(arena.map_chunks);
  zeroeth_chunk->ptr_to_mobiles = arena.mobiles;
 
  return arena;
}

MapChunk* newMapChunk(MapChunk* map_chunks){
 
  MapChunk src;

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

  src.blocks_sight_bmp = bitMapCreate(CHUNK_WIDTH, CHUNK_WIDTH);
  src.blocks_movement_bmp = bitMapCreate(CHUNK_WIDTH, CHUNK_WIDTH);

  const int size = CHUNK_WIDTH * CHUNK_WIDTH;
  src.terrain = createObjectBuffer(size);
  for(size_t i = 0; i < size; i++){
    objectPush(src.terrain, &air);
  }

  src.portals = NULL;
  src.ptr_to_mobiles = map_chunks[0].ptr_to_mobiles;
  return myBufferPush(map_chunks, &src, sizeof(MapChunk));
}

GameObject* newMobile(MapPosition dst){

  if(dst.chunk_ptr == NULL ||
     dst.chunk_ptr->ptr_to_mobiles == NULL){
      printf("no chunk to allocate from\n");
      return NULL;
  }

  GameObject proto_mob = {
    OBJECT_MOBILE,
    .unicode = 1,
    .atlas = 1,//ASCII_TEXTURE_INDEX,
    .fg = 15,
    .bg = 0,
    .type.mob = {
      .pos = dst
    },
    .inventory = createObjectBuffer(4)
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
  objectPush(proto_mob.inventory, &proto_item);
  
  //GameObjectBuffer mobiles = 
    //  getArenaFromChunk(dst.chunk_ptr)->mobiles;
  GameObjectBuffer mobiles = dst.chunk_ptr->ptr_to_mobiles;

  return objectPush(mobiles, &proto_mob);
}

GameObject getTerra(MapPosition pos){
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
     offset >= myBufferMeta(chunk->terrain)->top){
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

int setTerra(GameObject proto, MapPosition pos){
  if(proto.type_enum != OBJECT_TERRAIN) return 1;

  MapChunk* chunk = pos.chunk_ptr;
  uint32_t offset = (pos.y * CHUNK_WIDTH) + pos.x;  
  if(pos.x < 0 ||
     pos.x >= CHUNK_WIDTH ||
     offset >= myBufferMeta(chunk->terrain)->top){
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
static int round_ties_up(double n) {
      return (int)floor(n + 0.5);
}

static int round_ties_down(double n) {
   return (int)ceil(n - 0.5);
}

static Fraction slope(int row_depth, int col) {
    return fractionNew(2 * col - 1, 2 * row_depth);
}

static bool isSymmetric(Row* row, int col)
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

static void shadowcastMarkVisible(BitMap* shadow_mask, MapPosition pos) {
  bitMapSetPx(shadow_mask, pos.x, pos.y, 0);
}

static void shadowcastScanRow(BitMap* dst_mask, MapPosition camera, Row current_row){
  
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
    
    if(is_wall || isSymmetric(&current_row, col)) {
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
  
  int x_offset = camera.x - (gfxGetScreenWidth(gfx) / 2);
  int y_offset = camera.y - (gfxGetScreenHeight(gfx) / 2);

  BitMap* shadow_mask = shadowcastFOV(camera);

  GameObjectBuffer terrain = camera.chunk_ptr->terrain;
  struct MyBufferMeta info = *myBufferMeta(terrain);

  for(unsigned int i = 0; i < info.top; i++){
    int shadow_x = i % CHUNK_WIDTH;
    int shadow_y = i / CHUNK_WIDTH;
    if(bitMapGetPx(shadow_mask, shadow_x, shadow_y) == 0){
      GameObject tile = terrain[i];
      int tile_x = shadow_x - x_offset;
      int tile_y = shadow_y - y_offset;
      gfxRenderGlyph(gfx, tile_x, tile_y, tile.unicode, tile.atlas, tile.fg, tile.bg);
    }
    
  }

  GameObjectBuffer mobiles = camera.chunk_ptr->ptr_to_mobiles;
  //    getArenaFromChunk(camera.chunk_ptr)->mobiles;
  for(size_t i = 0; i < myBufferMeta(mobiles)->top; i++){
    
    GameObject actor = mobiles[i];
    MapPosition pos = actor.type.mob.pos;
    if(bitMapGetPx(shadow_mask, actor.type.mob.pos.x, actor.type.mob.pos.y) == 0){
      gfxRenderGlyph(gfx, pos.x - x_offset, pos.y - y_offset,
	     actor.unicode, actor.atlas,
	     actor.fg, actor.bg);
    }
  }

  bitMapDestroy(shadow_mask);
  
  return 0;
}
