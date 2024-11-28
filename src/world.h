#ifndef WORLD_H
#define WORLD_H
#include "vkterm/vkterm.h"
#include "bitmap.h"

#define CHUNK_WIDTH 32
#define CHUNK_OBJECT_C 8

typedef struct GameObject GameObject;
typedef struct GameObject* GameObjectBuffer;

struct GameObjectBufferInfo{
  size_t capacity;
  size_t count;
};

typedef struct{
  BitMap* blocks_sight_bmp;
  BitMap* blocks_movement_bmp;
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
  GameObjectType type_enum;
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
    
  } type;
  GameObjectBuffer inventory;
};

MapChunk* mapChunkCreate(void);
uint8_t terraBlocksMove(MapPosition pos);
uint8_t terraBlocksSight(MapPosition pos);
int mapSetTerrain(GameObject, MapPosition);
GameObject* mobileCreate(MapPosition);
BitMap* shadowcast_fov(MapPosition);
int mapChunkDraw(Gfx, MapPosition);

#endif // WORLD_H
