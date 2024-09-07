#include "world.h"

typedef struct {
  void (*func)(gameWorld, void*);
  void* arg;
} action_t;

#define DEFINE_ACTION_FUNC(action, arg_type)				\
  void action(gameWorld world, void* arg) {			\
    action##_f(world, *(arg_type*)arg);			\
  }

DEFINE_ACTION_FUNC(moveEntity, actionArgs_t);
