#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
//#include "config.h"
#include "action.h"

typedef enum{
  ARG_INT,
  ARG_FLOAT,
  ARG_STRING,
}ArgType;

typedef union{
  int i;
  float f;
  const char* s;
}ArgValue;

typedef struct {
  ArgType type;
  ArgValue val;
}Argument;

typedef int (*Action_f)(GameObject* object_ptr, int args_c, Argument* args);

struct GameActionImpl{
    Action_f func;
    int args_c;
    Argument* args;
};

GameAction requestAction(Action_f func, int args_c, ...){

  GameAction action = malloc(sizeof(struct GameActionImpl));
  action->func = func;
  action->args_c = args_c;
  action->args = malloc(sizeof(Argument) * args_c);

  va_list args;
  va_start(args, args_c);
  for(int i = 0; i < args_c; i++){
    // even number args specify type
    ArgType type = va_arg(args, ArgType);
    action->args[i].type = type;

    // odd number args specify type
    switch(type){
    case ARG_INT:
      action->args[i].val.i = va_arg(args, int);
      break;
    case ARG_FLOAT:
      action->args[i].val.f = (float)va_arg(args, double);
      break;
    case ARG_STRING:
      action->args[i].val.s = va_arg(args, const char*);
      break;
    }
   
  }
  va_end(args);
  //GameAction* action_ptr = malloc(sizeof(GameAction));
  //*action_ptr = action;
  return action;
}

void doAction(GameObject* object_ptr, GameAction action){
  int err = action->func(object_ptr, action->args_c, action->args);
  // segfault on _noAction
  if(err == 1){
    printf("action error\n");
  }
  free(action->args);
  free(action);
}

/* Start of in-game functions */
/* Arg 1 = ARG_INT, Arg2 entity Index */
int _buildTerrainAction(GameObject* object_ptr, int args_c, Argument* args){
  if(object_ptr == NULL) return 1;
  if(args_c > 1) return 1;
  int inventory_index = args[0].val.i;
  GameObject tool = object_ptr->inventory[inventory_index];

  tool.type_enum = OBJECT_TERRAIN;
  tool.type.terra.blocks_sight = 1;
  tool.type.terra.blocks_movement = 1;

  return mapSetTerrain(tool, object_ptr->type.mob.pos);
}
GameAction buildTerrainAction(int entity_index){
  return requestAction(_buildTerrainAction, 1,
		       ARG_INT, entity_index);
}

int _moveMobileAction(GameObject* object_ptr, int args_c, Argument* args){
  if(object_ptr == NULL) return 1;
  if(object_ptr->type_enum != OBJECT_MOBILE) return 1;
  if(args_c > 2) return 1;
  int dx = args[0].val.i;
  int dy = args[1].val.i;

  MapPosition pos = object_ptr->type.mob.pos;
  pos.x += dx;
  pos.y += dy;
  if(mapGetTerrain(pos).type.terra.blocks_movement == 0){
    object_ptr->type.mob.pos = pos;
  }

  return 0;
}
GameAction moveMobileAction(int x, int y){
  return requestAction(_moveMobileAction, 2,
                  ARG_INT, x,
                  ARG_INT, y);
}

int _paintEntityAction(GameObject* object_ptr, int args_c, Argument* args){
  if(object_ptr == NULL) return 1;
  if(args_c > 4) return 1;

  GameObject* item = &object_ptr->inventory[0];
  
  if(args[0].val.i >= 0)
    item->unicode = args[0].val.i;
  if(args[1].val.i >= 0)
    item->atlas = args[1].val.i;
  if(args[2].val.i >= 0)
    item->fg = args[2].val.i;
  if(args[3].val.i >= 0)
    item->bg = args[3].val.i;

  return 0;
}
GameAction paintEntityAction(int uv, int atlas, int fg, int bg){
  return requestAction(_paintEntityAction, 4,
		       ARG_INT, uv,
		       ARG_INT, atlas,
		       ARG_INT, fg,
		       ARG_INT, bg);
}

int _noAction(GameObject* object_ptr, int args_c, Argument* args){
  return 0;
}
GameAction noAction(void){
  return requestAction(_noAction, 0, 0);
}

//end
