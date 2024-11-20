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
  
  MapChunk* chunk = mapChunkCreate();
  MapPosition start_pos = { 2, 2, chunk };

  GameObject* player = mobileCreate(start_pos);
  player->unicode = 417;
  player->atlas = DRAW_TEXTURE_INDEX;
  
  gfxCacheChange(gfx, "ui");
  gfxAddString(gfx, 0, ASCII_SCREEN_HEIGHT-2,
	       "Press ? for help",
	       15, 0);
  gfxCacheChange(gfx, "main");
  
  while(getExitState() == 0){
    mapChunkDraw(gfx, player->data.mob.pos);

    GameAction user_action = gfxUserInput(gfx);
    doAction(player, user_action);
    
    gfxCachePresent(gfx, "main");
    gfxCachePresent(gfx, "ui");
    gfxRefresh(gfx);
  }
  
  gfxScreenClose(gfx);
  return 0;
}
