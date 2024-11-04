#include "vulkan_public.h"
#include "world.h"
#include "controls.h"
#include "action.h"
#include <stdio.h>
#include <stdlib.h>

int main(void){

  Gfx gfx = gfxScreenInit();
 
  gfxTextureLoad(gfx, "textures/color.png");
  gfxTextureLoad(gfx, "textures/icl8x8u.bdf");
  gfxTextureLoad(gfx, "textures/mrmotext-ex11.png");
  
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

  gfxCacheChange(gfx, "ui");
  gfxAddString(gfx, 0, ASCII_SCREEN_HEIGHT-2,
	       "Press ? for help",
	       15, 0);
  gfxCacheChange(gfx, "main");
  
  while(getExitState() == 0){
    worldDraw(gfx, world, *player);
    // move to gfxUserInput

    GameAction user_action = gfxUserInput(gfx, 0);
    // change gfxUserInput() to function pointer
    doAction(&world, user_action);
    
    gfxCachePresent(gfx, "main");
    gfxCachePresent(gfx, "ui");
    gfxRefresh(gfx);
  }
  
  gfxScreenClose(gfx);
  return 0;
}
