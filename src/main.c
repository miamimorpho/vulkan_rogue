#include "vulkan_public.h"
#include "input.h"
#include "world.h"
#include "action.h"
#include <stdio.h>

int main(void){
  if(gfxConstInit() != 0){
    printf("error");
  }
  gfxGlobalInit();
  inputInit();
  gfxTilesetLoad("textures/spleen-8x16-437-custom.bdf");

  GameWorld world;
  worldInit(&world, 16);

  entityAdd(&world, (Entity){
      .ch = 1,
      .pos= {1,1},
    });
    
  while(getExitState() == 0){
    
    gfxDrawStart();

    for(int i = 0; i < world.size; i++){
      Entity tile = world.tiles[i];
      gfxDrawChar(tile.ch, tile.pos.x, tile.pos.y);
    }
    for(int i = 0; i < world.actors_count; i++){
      Entity actor = world.actors[i];
      gfxDrawChar(actor.ch, actor.pos.x, actor.pos.y);
    }
    
    gfxDrawEnd();

    GameAction user_action = userInput();
    doAction(&world, user_action);

  }
  
  gfxClose();
  printf("success\n");
  return 0;
}
