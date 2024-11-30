#include "vkterm/vkterm.h"
#include "world.h"

extern double lMouseX(void){
  double x = 0;
  gfxMouseNorm(&x, NULL);
  return x;
}

extern double lMouseY(void){
  double y = 0;
  gfxMouseNorm(NULL, &y);
  return y;
}

extern int lGetUnicode(void){
  return gfxInputUnicode();
}

extern int lMoveMobile(GameObject* object_ptr, int dx, int dy){
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

