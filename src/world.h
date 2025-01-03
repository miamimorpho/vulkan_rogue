#ifndef WORLD_H
#define WORLD_H
#include "vkterm/vkterm.h"
#include "mystdlib.h"

#define CHUNK_WIDTH 24
#define CHUNK_SIZE CHUNK_WIDTH * CHUNK_WIDTH
#define CHUNK_OBJECT_C 8

typedef struct GameObject GameObject;
// TODO use bitmap for freelist and pool
//typedef struct GameObject* GameObjectBuffer;
// or hashmap MapChunk:mobiles


typedef struct MapChunk MapChunk;
//typedef struct MapChunk* MapChunkBuffer;

struct WorldArena{
   AllocatorInterface allocator;
   Bitmap* map_chunks_free;
   MapChunk* map_chunks;
   Bitmap* mobiles_free;
   GameObject* mobiles;
};

struct GameObjectTile{
    uint32_t unicode;
    uint8_t atlas;
    uint8_t fg;
    uint8_t bg;
};

struct MapChunk{
  struct GameObjectTile* terrain_tiles;
  Bitmap* blocks_sight_bmp;
  Bitmap* blocks_movement_bmp;
  struct WorldArena* ptr_to_arena;
};

typedef struct{
  int32_t x;
  int32_t y;
  MapChunk* chunk_ptr;
} MapPosition;

typedef enum {
    OBJECT_TERRAIN,
    OBJECT_MOBILE,
    OBJECT_ITEM,
} GameObjectType;

struct GameObject{
  GameObjectType type_enum;
  struct GameObjectTile tile;
  union {
    struct {
      uint8_t blocks_sight;
      uint8_t blocks_movement;
    } terra;
    
    struct {
      MapPosition pos;
    } mob;
    
    struct {
       int id;
    } item;
    
  } type;
  GameObject* inventory;
};

struct WorldArena* createWorldArena(AllocatorInterface);
uint8_t terraDoesBlockMove(MapPosition);
uint8_t terraDoesBlockSight(MapPosition);
int terraSet(GameObject, MapPosition);

GameObject* mobilePush(MapPosition);

void destroyWorldArena(struct WorldArena*);


int mapChunkDraw(Gfx, struct WorldArena*, MapPosition);

#endif // WORLD_H
