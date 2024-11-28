#ifndef ACTION_H
#define ACTION_H

#include "world.h"

extern int lMouseX(void);
extern int lMouseY(void);
extern int lGetUnicode(void);
extern int lMoveMobile(GameObject* object_ptr, int dx, int dy);

/*
struct GameActionImpl;
typedef struct GameActionImpl* GameAction;

GameAction moveMobileAction(int, int);
GameAction paintEntityAction(int, int, int, int);
GameAction buildTerrainAction(int);
GameAction noAction(void);
void doAction(GameObject*, GameAction);
*/

#endif // ACTION_H
