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

  gfxTextureLoad("textures/spleen-8x16-437-custom.bdf");
  gfxTextureLoad("textures/ic16x16u.bdf");
  
  GameWorld world;
  worldInit(&world, 16, 32);

  entityAdd(&world, (Entity){
      .unicode = 1,
      .color = 0xFFFFFF,
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
	gfxDrawChar(tile.unicode, x, y,
		    tile.color, 1);
      }
    }

    // render list of entities (actors)
    for(int i = 0; i < world.actors_count; i++){
      Entity actor = world.actors[i];
      gfxDrawChar(actor.unicode,
		  ASCII_SCREEN_WIDTH / 2,
		  ASCII_SCREEN_HEIGHT / 2,
		  actor.color, 0);
    }

    // user interface
    gfxDrawString("AAAhello world", 0, 0, 0xFFFFF);

    gfxDrawEnd();

    GameAction user_action = userInput(0);
    doAction(&world, user_action);

  }
  
  gfxClose();
  printf("success\n");
  return 0;
}
