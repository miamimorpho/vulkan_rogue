#include "vkterm/vkterm.h"
#include "world.h"
#include "action.h"
#include "lua_interface.h"
#include <stdio.h>
#include <stdlib.h>

int main(void){

  Gfx gfx = gfxScreenInit();

  loadLuaConfigTextures(gfx);

  lua_State* L = loadLuaConfigControls();

  MapChunk* chunk = mapChunkCreate();
  MapPosition start_pos = { 2, 2, chunk };

  GameObject* player = mobileCreate(start_pos);
  player->unicode = 417;
  player->atlas = 2;
  
  gfxCacheChange(gfx, "ui");
  //gfxAddString(gfx, 0, ASCII_SCREEN_HEIGHT-2,
  //	       "Press ? for help",
  //	       15, 0);
  gfxCacheChange(gfx, "main");
  
  while(getExitState() == 0){
    mapChunkDraw(gfx, player->type.mob.pos);

    gfxPollEvents(gfx);
    objectLuaRunScript(L, player);
    
    gfxCachePresent(gfx, "main");
    gfxCachePresent(gfx, "ui");
    gfxRefresh(gfx);
  }
  
  gfxScreenClose(gfx);
  return 0;
}
