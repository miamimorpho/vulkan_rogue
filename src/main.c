#include "vulkan_public.h"
#include "input.h"
#include "world.h"
#include "action.h"
#include <stdio.h>
#include <stdlib.h>

int main(void){

  gfxScreenInit();
  inputInit();

  gfxTextureLoad("textures/color.png");
  gfxTextureLoad("textures/icl8x8u.bdf");
  gfxTextureLoad("textures/mrmotext-ex11.png");
  
  GameWorld world;
  worldInit(&world, 16, 16);

  Entity* player = entityInit(&world, 0);
  player->uv = 417;
  player->inventory_c = 1;
  player->inventory = malloc(sizeof(Entity));
  player->inventory[0].fg = 15;
  player->inventory[0].bg = 0;
  player->inventory[0].uv = 1;
  player->inventory[0].collide = 0;

  while(getExitState() == 0){
    worldDraw(world, *player);

    GameAction user_action = userInput(0);
    doAction(&world, user_action);
    
    //gfxAddString(0, ASCII_SCREEN_HEIGHT-2,
    //		 "Press ? for help",
    //		 15, 0);
    gfxCachePresent("main");
    gfxRefresh();
  }
  
  gfxScreenClose();
  return 0;
}
