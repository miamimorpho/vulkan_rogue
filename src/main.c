#include "vkterm/vkterm.h"
#include "world.h"
#include "action.h"
#include "lua_interface.h"
#include <stdio.h>
#include <stdlib.h>

int main(void){

  Gfx gfx = gfxScreenInit();
  luaLoadTextures(gfx);

  WorldArena arena = createWorldArena();

  lua_State* L = luaStateInit(gfx, "lua/controls.lua");

  MapPosition start_pos = { 2, 2, arena.map_chunks };
  
  GameObject* player = newMobile(start_pos);
  if(player == NULL) printf("err\n");
  player->unicode = 417;
  player->atlas = 2;
  
  gfxLayerChange(gfx, "ui");
  gfxRenderElement(gfx, 0, 0, "Press ? for help", 1, 15, 0);
  gfxLayerChange(gfx, "main");
  
  while(getExitState() == 0){
    mapChunkDraw(gfx, player->type.mob.pos);

    gfxPollEvents(gfx);
    luaControls(L, player);
    
    gfxLayerPresent(gfx, "main");
    gfxLayerPresent(gfx, "ui");
    gfxRefresh(gfx);
  }
  
  gfxScreenClose(gfx);
  return 0;
}
