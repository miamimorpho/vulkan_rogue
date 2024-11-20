#ifndef ACTION_H
#define ACTION_H

#include "world.h"

struct GameActionImpl;
typedef struct GameActionImpl* GameAction;

GameAction moveMobileAction(int, int);
GameAction paintEntityAction(int, int, int);
GameAction buildTerrainAction(int);
GameAction noAction(void);
void doAction(GameObject*, GameAction);

#endif // ACTION_H
