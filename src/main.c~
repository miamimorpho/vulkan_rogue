#include "vulkan_public.h"
#include "input.h"
#include "world.h"
#include "action.h"
#include <stdio.h>
#include <stdlib.h>

int main(void){
  if(gfxConstInit() != 0){
    printf("error");
  }
  gfxGlobalInit();
  inputInit();

  gfxTextureLoad("textures/color.png");
  gfxTextureLoad("textures/icl8x8u.bdf");
  gfxTextureLoad("textures/mrmotext-ex11.png");
  
  GameWorld world;
  worldInit(&world, 16, 16);

  Entity* player = entityInit(&world, 0);
  player->uv = 417;
  player->inventory_c = 1;
  player->inventory = malloc(sizeof(Entity));
  player->inventory[0].fg = 15;
  player->inventory[0].bg = 0;
  player->inventory[0].uv = 1;
  player->inventory[0].collide = 0;
     
  while(getExitState() == 0){
    
    gfxDrawStart();

    // render world map
    Pos camera = player->pos;
    camera.x -= ASCII_SCREEN_WIDTH / 2;
    camera.y -= ASCII_SCREEN_HEIGHT / 2;
    for(int y = 0; y < ASCII_SCREEN_HEIGHT; y++){
      for(int x = 0; x < ASCII_SCREEN_WIDTH; x++){
	Entity tile = mapGetTile(&world, camera.x +x, camera.y +y);
	gfxDrawChar(tile.uv, x, y,
		    tile.fg, tile.bg, DRAW_TEXTURE_INDEX);
      }
    }

    // render list of entities (actors)
    for(uint i = 0; i < world.actors_count; i++){
      Entity actor = world.actors[i];
      if(actor.is_init == 1){
	gfxDrawChar(actor.uv,
		    ASCII_SCREEN_WIDTH / 2,
		    ASCII_SCREEN_HEIGHT / 2,
		    actor.fg, actor.bg, DRAW_TEXTURE_INDEX);
      }
    }

    // user interface
    //gfxDrawString("hello world", 0, 0, 0xFFFFFFFF, 0xFF000000);

    gfxDrawEnd();

    GameAction user_action = userInput(0);
    doAction(&world, user_action);

  }
  
  gfxClose();
  printf("success\n");
  return 0;
}
