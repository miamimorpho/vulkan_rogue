#include "world.h"
#include "maths.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mystdlib.h"

GameObject* getMobilesFromMapChunk(MapChunk* chunk){
    // TODO: MapChunks and Mobiles form a linked list of arenas
    return chunk->ptr_to_arena->mobiles;
}

MapChunk mapChunkInit(struct WorldArena* ptr_to_arena){
 
  AllocatorInterface allocator = ptr_to_arena->allocator;

  MapChunk src;
  src.blocks_sight_bmp = bitmapCreate(CHUNK_WIDTH, CHUNK_WIDTH, allocator);
  src.blocks_movement_bmp = bitmapCreate(CHUNK_WIDTH, CHUNK_WIDTH, allocator);
  
  struct GameObjectTile air = {
    .unicode = 0,
    .atlas = 2,//DRAW_TEXTURE_INDEX
    .fg = 15,
    .bg = 3,
  };
  const size_t tile_c = CHUNK_WIDTH * CHUNK_WIDTH;
  src.terrain_tiles = memSliceCreate(tile_c, sizeof(struct GameObjectTile), allocator);
  for(size_t i = 0; i < tile_c; i++){
      src.terrain_tiles[i] = air;
  }

  src.ptr_to_arena = ptr_to_arena;
  return src;
}

GameObject mobileInit(AllocatorInterface allocator){

  GameObject proto_mob = {
    OBJECT_MOBILE,
    .tile.unicode = 1,
    .tile.atlas = 1,//ASCII_TEXTURE_INDEX,
    .tile.fg = 15,
    .tile.bg = 0,
    .inventory = memSliceCreate(4, sizeof(GameObject), allocator)
  };
  GameObject proto_item = {
    OBJECT_ITEM,
    .tile.unicode = 0,
    .tile.atlas = 1,//ASCII_TEXTURE_INDEX,
    .tile.fg = 15,
    .tile.bg = 0,
    .type.item = {
      .id = 4
    },
    .inventory = NULL,
  };
  proto_mob.inventory[0] = proto_item;
  return proto_mob;
}

struct WorldArena* createWorldArena(AllocatorInterface allocator){
  struct WorldArena* arena = allocator.mallocFn(allocator.ctx, sizeof(struct WorldArena));
  arena->allocator = allocator;

  const int mobile_c = 10;
  arena->mobiles_free = bitmapCreate(mobile_c, 1, allocator);
  arena->mobiles = memSliceCreate(mobile_c, sizeof(GameObject), allocator);
  for(int i = 0; i < mobile_c; i++){
      arena->mobiles[i] = mobileInit(allocator);
  }
  
  const int chunk_c = 10;
  arena->map_chunks_free = bitmapCreate(chunk_c, 1, allocator);
  arena->map_chunks = memSliceCreate(chunk_c, sizeof(MapChunk), allocator);
  for(int i = 0; i < chunk_c; i++){
      arena->map_chunks[i] = mapChunkInit(arena);
  }

  return arena;
}

GameObject* mobilePush(MapPosition pos){

    struct WorldArena* a = pos.chunk_ptr->ptr_to_arena;

    int64_t free_index = -1;
    
    int64_t mobile_c = memSliceSize(a->mobiles) / sizeof(GameObject);
    printf("%lld\n", mobile_c);
    for(int i = 0; i < mobile_c; i++){
        if(bitmapGetPx(a->mobiles_free, i, 0) == 0){
            free_index = i;
            break;
        }
    }
    if(free_index < 0){
        fprintf(stderr, "no free memory slot found");
        return NULL;
    }
    a->mobiles[free_index].type.mob.pos = pos;
    bitmapSetPx(a->mobiles_free, free_index, 0, 1);
    return &a->mobiles[free_index];
}

void destroyWorldArena(struct WorldArena* arena){
    memArenaDestroy(arena->allocator.ctx);
}

GameObject getTerra(MapPosition pos){
  MapChunk* chunk = pos.chunk_ptr;
  
  GameObject null_object = {
    OBJECT_TERRAIN,

    .tile.unicode = 370,
    .tile.atlas = 2,//DRAW_TEXTURE_INDEX,
    .tile.fg = 3,
    .tile.bg = 0,

    .type.terra = {
      .blocks_movement = 0,
      .blocks_sight = 1,
    }
  };

  uint32_t offset = (pos.y * CHUNK_WIDTH) + pos.x;  
  if(pos.x < 0 ||
     pos.x >= CHUNK_WIDTH ||
     pos.y < 0 ||
     pos.y >= CHUNK_WIDTH){
    return null_object;
  }

  return (GameObject){
      OBJECT_TERRAIN,
      .tile = chunk->terrain_tiles[offset],
      .type.terra = {
          .blocks_movement = terraDoesBlockMove(pos),
          .blocks_sight = terraDoesBlockSight(pos),
      }      
  };
      
}

uint8_t terraDoesBlockMove(MapPosition pos){
  MapChunk* chunk = pos.chunk_ptr;
  return bitmapGetPx(chunk->blocks_movement_bmp, pos.x, pos.y);
}

uint8_t terraDoesBlockSight(MapPosition pos) {
  MapChunk* chunk = pos.chunk_ptr;
  return bitmapGetPx(chunk->blocks_sight_bmp, pos.x, pos.y);
}

int terraSet(GameObject proto, MapPosition pos){
  if(proto.type_enum != OBJECT_TERRAIN) return 1;

  MapChunk* chunk = pos.chunk_ptr;
  uint32_t offset = (pos.y * CHUNK_WIDTH) + pos.x;  
  if(pos.x < 0 ||
     pos.x >= CHUNK_WIDTH ||
     pos.y < 0 ||
     pos.y >= CHUNK_WIDTH){
    return 1;
  }

  if(proto.type.terra.blocks_sight == 1){
    bitmapSetPx(chunk->blocks_sight_bmp, pos.x, pos.y, 1);
  }
  if(proto.type.terra.blocks_movement == 1){
    bitmapSetPx(chunk->blocks_movement_bmp, pos.x, pos.y, 1);
  }

  proto.type_enum = OBJECT_ITEM;
  proto.type.item.id = 0;
  chunk->terrain_tiles[offset] = proto.tile;
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

static void shadowcastMarkVisible(Bitmap* shadow_mask, MapPosition pos) {
  bitmapSetPx(shadow_mask, pos.x, pos.y, 0);
}

static void shadowcastScanRow(Bitmap* dst_mask, MapPosition camera, Row current_row){

  static float SHADOWCAST_MAX = 12.5 * 12.5;

  // allows early termination on invalid angles
  if (fractionCompare(current_row.end_slope, current_row.start_slope)) {
    return;
  }

  // depth * slope
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
    uint8_t is_wall = terraDoesBlockSight(current_pos);
    
    if(is_wall || isSymmetric(&current_row, col)) {
      if(relativeDistance(current_row.depth, col) < SHADOWCAST_MAX)
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

Bitmap* shadowcastFOV(Bitmap* shadow_mask, MapPosition camera){

  bitmapFill(shadow_mask, 1);
  
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

int mapChunkDraw(Gfx gfx, struct WorldArena* arena, MapPosition camera){

  gfxClear(gfx);
  
  int x_offset = camera.x - (gfxGetScreenWidth(gfx) / 2);
  int y_offset = camera.y - (gfxGetScreenHeight(gfx) / 2);

  AllocatorInterface allocator = arena->allocator;
  Bitmap* shadow_mask = bitmapCreate(128, 128, allocator);
  shadow_mask = shadowcastFOV(shadow_mask, camera);

  MapChunk ch = *camera.chunk_ptr;
  struct GameObjectTile* terrain = ch.terrain_tiles;
 
  for(int i = 0; i < CHUNK_SIZE; i++){
    int shadow_x = i % CHUNK_WIDTH;
    int shadow_y = i / CHUNK_WIDTH;
    if(bitmapGetPx(shadow_mask, shadow_x, shadow_y) == 0){
      struct GameObjectTile t = terrain[i];
      int tile_x = shadow_x - x_offset;
      int tile_y = shadow_y - y_offset;
      //printf("%d - t.fg %d \n", i ,t.fg);
      gfxRenderGlyph(gfx, tile_x, tile_y, 
                     t.unicode, t.atlas, t.fg, t.bg);
    }    
  }

  GameObject* mobiles = arena->mobiles;
  for(size_t i = 0; i < memSliceSize(mobiles) / sizeof(GameObject); i++){
    if(bitmapGetPx(arena->mobiles_free, i, 0) == 1){
        GameObject actor = mobiles[i];
        MapPosition pos = actor.type.mob.pos;
        if(bitmapGetPx(shadow_mask, actor.type.mob.pos.x, actor.type.mob.pos.y) == 0){
            gfxRenderGlyph(gfx, pos.x - x_offset, pos.y - y_offset,
                           actor.tile.unicode, actor.tile.atlas,
                           actor.tile.fg, actor.tile.bg);
        }
    }
  }

  bitmapDestroy(shadow_mask, allocator);
  
  return 0;
}
