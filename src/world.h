#ifndef WORLD_H
#define WORLD_H
#include "vulkan_public.h"
#include <stdint.h>

#define CHUNK_WIDTH 32
#define CHUNK_OBJECT_C 8

typedef struct GameObject GameObject;
typedef struct GameObject* GameObjectBuffer;

#define BUFFER_METADATA(buffer) \
  ((size_t*)(buffer) - 2)
static inline size_t* bufferCapacity(GameObjectBuffer b){
  return &(BUFFER_METADATA(b)[0]);
}
static inline size_t* bufferSize(GameObjectBuffer b){
  return &(BUFFER_METADATA(b)[1]);
}

typedef struct{
  GameObjectBuffer terrain;
  GameObjectBuffer mobiles;
} MapChunk;

typedef struct{
  int32_t x;
  int32_t y;
  MapChunk* chunk_ptr;
} MapPosition;

typedef enum {
    OBJECT_TERRAIN,
    OBJECT_MOBILE,
    OBJECT_ITEM,
    // Add other types as needed
} GameObjectType;

struct GameObject{
  GameObjectType type;
  uint32_t unicode;
  uint32_t atlas;
  uint32_t fg;
  uint32_t bg;
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
    
  } data;
  GameObjectBuffer inventory;
};

MapChunk* mapChunkCreate(void);
GameObject mapGetTerrain(MapPosition);
int mapSetTerrain(GameObject, MapPosition);
GameObject* mobileCreate(MapPosition);
GameObjectBuffer shadowcast_fov(MapPosition);
int mapChunkDraw(Gfx, MapPosition);

#endif // WORLD_H
