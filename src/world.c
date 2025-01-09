#include "world.h"
#include "maths.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mystdlib.h"

const MapPosition NULL_POS = {0};
const struct GameObjectTile NULL_OBJECT_TILE = {
    .unicode = 370,
    .atlas = 2,
    .fg = 3,
    .bg = 0,
};

int mapChunkFillPaint(MapChunk target, struct GameObjectTile val){
    for(size_t i = 0; i < CHUNK_SIZE; i++){
        target.terrain_tiles[i] = val;
    }
    return 0;
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
  mapChunkFillPaint(src, air);

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

struct WorldArena* createWorldArena(void){
  AllocatorInterface allocator = memArenaCreate(MB);

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

  
  struct GameObjectTile air = {
    .unicode = 0,
    .atlas = 2,//DRAW_TEXTURE_INDEX
    .fg = 3,
    .bg = 3,
  };

  for(int i = 0; i < chunk_c; i++){
      arena->map_chunks[i] = mapChunkInit(arena);
      air.fg += i;
      air.bg += i;
      mapChunkFillPaint(arena->map_chunks[i], air);    
  }

  const size_t portal_c = 4;
  arena->portals = memSliceCreate(portal_c, sizeof(struct MapPortal), allocator);

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
    memArenaDestroy(arena->allocator);
}

uint8_t terraDoesBlockMove(MapPosition pos){
  MapChunk* chunk = pos.chunk_ptr;
  return bitmapGetPx(chunk->blocks_movement_bmp, pos.x, pos.y);
}

uint8_t terraDoesBlockSight(MapPosition pos) {
  MapChunk* chunk = pos.chunk_ptr;
  return bitmapGetPx(chunk->blocks_sight_bmp, pos.x, pos.y);
}

struct GameObjectTile terraGetTile(MapPosition pos) {
    if(pos.chunk_ptr == NULL ||
       pos.x < 0 ||
       pos.x >= CHUNK_WIDTH ||
       pos.y < 0 ||
       pos.y >= CHUNK_WIDTH){
        return NULL_OBJECT_TILE;
    }
    int offset = pos.y * CHUNK_WIDTH + pos.x;
    return pos.chunk_ptr->terrain_tiles[offset];
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

struct MapPortal portalAtPos(MapPosition pos){

    struct WorldArena* a = pos.chunk_ptr->ptr_to_arena;

    for(size_t i = 0; i < memSliceSize(a->portals) / sizeof(struct MapPortal); i++){
        struct MapPortal port = a->portals[i];
        if(port.src.chunk_ptr == pos.chunk_ptr &&
           port.src.x == pos.x &&
           port.src.y == pos.y) return port;
    }
    return (struct MapPortal){0};
}
