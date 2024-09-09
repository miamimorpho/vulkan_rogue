#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "action.h"

/* Parameters
 * arg_c 3
 * 0 int entity_ndex;
 * 1 int vector_x
 * 2 int vector_y
 */
int moveEntityAction(GameWorld* w, int args_c, Argument* args){
  if(w == NULL) return 1;
  if(args_c > 3) return 1;
  int entity_index = args[0].val.i;
  int vector_x = args[1].val.i;
  int vector_y = args[2].val.i;
  
  Entity* e = &w->actors[entity_index];
  e->pos.x += vector_x;
  e->pos.y += vector_y;

  return 0;
}

int noAction(GameWorld* w, int args_c, Argument* args){
  return 0;
}

GameAction requestAction(Action_f func, int args_c, ...){

  GameAction action;
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
  return action;
}

void doAction(GameWorld* w, GameAction* action){
  int err = action->func(w, action->args_c, action->args);
  if(err == 1){
    printf("action error\n");
  }
}
