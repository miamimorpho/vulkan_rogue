#ifndef ACTION_H
#define ACTION_H

#include "world.h"

typedef enum{
  ARG_INT,
  ARG_FLOAT,
  ARG_STRING,
}ArgType;

typedef union{
  int i;
  float f;
  const char* s;
  void* ptr;
}ArgValue;

typedef struct {
  ArgType type;
  ArgValue val;
}Argument;

typedef int (*Action_f)(GameWorld* w, int args_c, Argument* args);

typedef struct {
    Action_f func;
    int args_c;
    Argument* args;
}GameAction;

int moveEntityAction(GameWorld*, int, Argument*);
int         noAction(GameWorld*, int, Argument*);

GameAction requestAction(Action_f, int args_c, ...);
void doAction(GameWorld*, GameAction*);

#endif // ACTION_H
