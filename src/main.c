#include "vulkan_public.h"
#include "input.h"
#include "world.h"
#include <stdio.h>

int main(void){
  if(gfxConstInit() != 0){
    printf("error");
  }
  gfxGlobalInit();
  inputInit();
  gfxTilesetLoad("textures/spleen-8x16-437-custom.bdf");

  gameWorld_t world;
  worldInit(&world, 16);

  entityAdd(&world, (entity_t){
      .ch = 1,
      .pos= {1,1},
    });
    
  while(userInput() == 0){
    gfxDrawStart();

    for(int i = 0; i < world.size; i++){
      entity_t tile = world.tiles[i];
      gfxDrawChar(tile.ch, tile.pos.x, tile.pos.y);
    }
    for(int i = 0; i < world.actors_count; i++){
      entity_t actor = world.actors[i];
      gfxDrawChar(actor.ch, actor.pos.x, actor.pos.y);
    }
    
    gfxDrawEnd();
  }
  
  gfxClose();
  printf("success\n");
  return 0;
}
