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
  worldInit(&world, 32);

  entityAdd(&world, (Entity){
      .ch = 1,
      .pos= {1,1},
    });
    
  while(getExitState() == 0){
    
    gfxDrawStart();

    // render world map
    Pos camera = world.actors[0].pos;
    camera.x -= ASCII_SCREEN_WIDTH / 2;
    camera.y -= ASCII_SCREEN_HEIGHT / 2;
    for(int y = 0; y < ASCII_SCREEN_HEIGHT; y++){
      for(int x = 0; x < ASCII_SCREEN_WIDTH; x++){
	Entity tile = mapGetTile(&world, camera.x +x, camera.y +y);
	gfxDrawChar(tile.ch, x, y,
		    (ivec3){255,100,100});
      }
    }

    // render list of entities (actors)
    for(int i = 0; i < world.actors_count; i++){
      Entity actor = world.actors[i];
      gfxDrawChar(actor.ch,
		  ASCII_SCREEN_WIDTH / 2,
		  ASCII_SCREEN_HEIGHT / 2,
		  (ivec3){255, 255, 255});
    }
    
    gfxDrawEnd();

    GameAction user_action = userInput(0);
    doAction(&world, user_action);

  }
  
  gfxClose();
  printf("success\n");
  return 0;
}
