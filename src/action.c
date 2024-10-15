#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "config.h"
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

typedef int (*Action_f)(GameWorld* w, int args_c, Argument* args);

struct _GameAction{
    Action_f func;
    int args_c;
    Argument* args;
};

GameAction requestAction(Action_f func, int args_c, ...){

  struct _GameAction action;
  action.func = func;
  action.args_c = args_c;
  action.args = malloc(sizeof(Argument) * args_c);

  va_list args;
  va_start(args, args_c);
  for(int i = 0; i < args_c; i++){
    // even number args specify type
    ArgType type = va_arg(args, ArgType);
    action.args[i].type = type;

    // odd number args specify type
    switch(type){
    case ARG_INT:
      action.args[i].val.i = va_arg(args, int);
      break;
    case ARG_FLOAT:
      action.args[i].val.f = (float)va_arg(args, double);
      break;
    case ARG_STRING:
      action.args[i].val.s = va_arg(args, const char*);
      break;
    }
   
  }
  va_end(args);
  struct _GameAction* action_ptr = malloc(sizeof(struct _GameAction));
  *action_ptr = action;
  return action_ptr;
}

void doAction(GameWorld* w, GameAction action){
  int err = action->func(w, action->args_c, action->args);
  // segfault on _noAction
  if(err == 1){
    printf("action error\n");
  }
  free(action->args);
  free(action);
}

/* Start of in-game functions */
int _dropAction(GameWorld* w, int args_c, Argument* args){
  if(w == NULL) return 1;
  if(args_c > 1) return 1;
  int x = w->actors[0].pos.x;
  int y = w->actors[0].pos.y;

  Entity target = w->actors[0].inventory[0];
  
  mapPutTile(w, target, x, y);
  return 0;
}
GameAction dropAction(int entity_index){
  return requestAction(_dropAction, 1,
		       ARG_INT, entity_index);
}

int _moveEntityAction(GameWorld* w, int args_c, Argument* args){
  if(w == NULL) return 1;
  if(args_c > 3) return 1;
  int entity_index = args[0].val.i;
  int vx = args[1].val.i;
  int vy = args[2].val.i;
  
  Entity* e = &w->actors[entity_index];
  Entity dst = mapGetTile(w, e->pos.x + vx, e->pos.y + vy);
  if(dst.collide == 0){
    e->pos.x += vx;
    e->pos.y += vy;
  }

  return 0;
}
GameAction moveEntityAction(int entity_index, int x, int y){
  return requestAction(_moveEntityAction, 3,
                  ARG_INT, entity_index,
                  ARG_INT, x,
                  ARG_INT, y);
}

int _paintEntityAction(GameWorld* w, int args_c, Argument* args){
  if(w == NULL) return 1;
  if(args_c > 3) return 1;

  Entity* dst = &w->actors[0].inventory[0];
  if(args[0].val.i >= 0)
    dst->uv = args[0].val.i;
  if(args[1].val.i >= 0)
    dst->fg = args[1].val.i;
  if(args[2].val.i >= 0)
    dst->bg = args[2].val.i;

  return 0;
}

GameAction paintEntityAction(int uv, int fg, int bg){
  return requestAction(_paintEntityAction, 3,
		       ARG_INT, uv,
		       ARG_INT, fg,
		       ARG_INT, bg);
}

int _noAction(GameWorld* w, int args_c, Argument* args){
  return 0;
}
GameAction noAction(void){
  return requestAction(_noAction, 1, 1);
}
