#include "vkterm/vkterm.h"
#include "world.h"
#include "action.h"
#include "lua_interface.h"
#include "fov.h"
#include <stdio.h>
#include <stdlib.h>

int main(void){
  // vulkan init
  Gfx gfx = gfxScreenInit();
  luaLoadTextures(gfx);
  lua_State* L = luaStateInit(gfx, "lua/controls.lua");

  // world init
  struct WorldArena* arena = createWorldArena();
  MapPosition start_pos = { 2, 2, arena->map_chunks };
  GameObject* player = mobilePush(start_pos);
  player->tile.unicode = 417;
  player->tile.atlas = 2;

  MapPosition src = { 3, 3, &arena->map_chunks[0] };
  MapPosition dst = { 10, 10, &arena->map_chunks[1] };
  arena->portals[0] = (struct MapPortal){ src, dst };
  
  // ui init
  gfxLayerChange(gfx, "ui");
  gfxRenderElement(gfx, 0, 0, "Press ? for help", 1, 15, 0);
  gfxLayerChange(gfx, "main");
  
  AllocatorInterface fov_allocator = memArenaCreate(1 * MB);
  while(getExitState() == 0){
    cameraDrawWorld(gfx, player->type.mob.pos, fov_allocator);

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
