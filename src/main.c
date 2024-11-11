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
  player->inventory.count = 1;
  Entity* holding = &player->inventory.data[0];
  holding->fg = 15;
  holding->bg = 0;
  holding->uv = 1;
  holding->collide = 1;
  holding->blocks_sight = 1;

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
