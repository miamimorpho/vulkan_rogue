#include "vkterm/vkterm.h"
#include "world.h"
#include "action.h"
#include "lua_interface.h"
#include "fov.h"
#include <stdio.h>
#include <stdlib.h>

int main(void){
  Gfx gfx = gfxScreenInit();
  luaLoadTextures(gfx);
  lua_State* L = luaStateInit(gfx, "lua/controls.lua");
  AllocatorInterface allocator = memArenaCreate(MB);

  struct WorldArena* arena = createWorldArena(allocator);
  MapPosition start_pos = { 2, 2, arena->map_chunks };
  GameObject* player = mobilePush(start_pos);
  player->tile.unicode = 417;
  player->tile.atlas = 2;

  MapPosition a = { 3, 3, &arena->map_chunks[0] };
  MapPosition b = { 1, 1, &arena->map_chunks[1] };
  arena->portals[0] = (struct MapPortal){ a, b };
  
  gfxLayerChange(gfx, "ui");
  gfxRenderElement(gfx, 0, 0, "Press ? for help", 1, 15, 0);
  gfxLayerChange(gfx, "main");
  
  while(getExitState() == 0){
    mapChunkDraw(gfx, arena, player->type.mob.pos);

    gfxPollEvents(gfx);
    luaControls(L, player);
    
    gfxLayerPresent(gfx, "main");
    gfxLayerPresent(gfx, "ui");
    gfxRefresh(gfx);
  }
  
  destroyWorldArena(arena);
  gfxScreenClose(gfx);
  return 0;
}
