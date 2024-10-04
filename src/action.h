#ifndef ACTION_H
#define ACTION_H

#include "world.h"

typedef struct _GameAction* GameAction;

GameAction moveEntityAction(int, int, int);
GameAction dropAction(int);
GameAction pickTileAction(int, int);
GameAction noAction(void);
void doAction(GameWorld*, GameAction);

#endif // ACTION_H
