#include "world.h"

extern int rogueMoveMobile(GameObject* object_ptr, int dx, int dy){
  if(object_ptr == NULL) return 1;
  if(object_ptr->type_enum != OBJECT_MOBILE) return 1;

  MapPosition pos = object_ptr->type.mob.pos;
  pos.x += dx;
  pos.y += dy;

  if(terraBlocksSight(pos) == 0){
    object_ptr->type.mob.pos = pos;
  }
  return 0;
}

