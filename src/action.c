#include "world.h"

int rogueMoveMobile(GameObject* object_ptr, int dx, int dy){
  if(object_ptr == NULL) return 1;
  if(object_ptr->type_enum != OBJECT_MOBILE) return 1;

  MapPosition pos = object_ptr->type.mob.pos;
  pos.x += dx;
  pos.y += dy;

  if(terraBlocksMove(pos) == 0){
    object_ptr->type.mob.pos = pos;
  }
  return 0;
}

int roguePaintObject(GameObject* object_ptr, uint16_t inventory_index, uint16_t unicode, uint16_t atlas, uint16_t fg, uint16_t bg){

  if(object_ptr == NULL) return 1;
  GameObject* item = &object_ptr->inventory[inventory_index];
  item->unicode = unicode;
  item->atlas = atlas;
  item->fg = fg;
  item->bg = bg;

  return 0;
}

int rogueBuildObject(GameObject* object_ptr, uint16_t inventory_index){
  if(object_ptr == NULL) return 1;
  GameObject tool = object_ptr->inventory[inventory_index];

  tool.type_enum = OBJECT_TERRAIN;
  tool.type.terra.blocks_sight = 1;
  tool.type.terra.blocks_movement = 1;

  setTerra(tool, object_ptr->type.mob.pos);
  return 0;
}
