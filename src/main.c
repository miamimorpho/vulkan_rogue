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

  gfxTextureLoad("textures/ic8x8u.bdf");
  gfxTextureLoad("textures/mrmotext-ex11.png");
  
  GameWorld world;
  worldInit(&world, 16, 16);

  entityAdd(&world, (Entity){
      .uv = '@',
      .color = 0xFFFFFF,
      .pos= {1,1},
      .collide = 1,

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
	gfxDrawChar(tile.uv, x, y,
		    tile.color, 1);
      }
    }

    // render list of entities (actors)
    for(int i = 0; i < world.actors_count; i++){
      Entity actor = world.actors[i];
      gfxDrawChar(actor.uv,
		  ASCII_SCREEN_WIDTH / 2,
		  ASCII_SCREEN_HEIGHT / 2,
		  actor.color, 0);
    }

    // user interface
    gfxDrawString("hello world", 0, 0, 0xFFFFF);

    gfxDrawEnd();

    GameAction user_action = userInput(0);
    doAction(&world, user_action);

  }
  
  gfxClose();
  printf("success\n");
  return 0;
}
