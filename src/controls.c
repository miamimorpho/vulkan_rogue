#include "controls.h"
#include "key_codes.h"
#include "action.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

GameAction guiPickTile(Gfx gfx)
{
  int width_in_tiles = 32;

  gfxCacheChange(gfx, "background");
  int viewport_size = 16;
  for(int y = 0; y < viewport_size; y++){
    for(int x = 0; x < viewport_size; x++){
      int i = y * width_in_tiles + x;
      uint32_t bg = 0;
      if((x % 2) - (y % 2) == 0) bg = 1;
      gfxAddCh(gfx, x, y, i, DRAW_TEXTURE_INDEX,
	       15, bg);
    }// end of x loop
  }// end of y loop
  
  uint32_t target_uv = 0;
 
  // GUI Render Loop
  gfxCacheChange(gfx, "main");
  for(uint32_t c = 0; c == 0; c = gfxInputUnicode()){

    int mouse_x, mouse_y;
    gfxMousePos(&mouse_x, &mouse_y);
    
    gfxClear(gfx);
    target_uv = (mouse_y * width_in_tiles) + mouse_x;

    if(mouse_x >= 0 && mouse_x < viewport_size &&
       mouse_y >= 0 && mouse_y < viewport_size){
      gfxAddCh(gfx, mouse_x, mouse_y,
	       target_uv, DRAW_TEXTURE_INDEX,
	       11, 0);
    }
 
    gfxCachePresent(gfx, "background");
    gfxCachePresent(gfx, "main");
    gfxRefresh(gfx);
    gfxPollEvents(gfx);
  } // end of GUI loop
  return paintEntityAction(target_uv, -1, -1);
}

int userInputHelpScreen(Gfx gfx)
{
  char* help_screen_ascii = configReadFile("data/controls.txt");
  gfxAddString(gfx, 0, 0, help_screen_ascii, 15, 0);

 
  for(uint32_t c = 0; c == 0; c = gfxInputUnicode()){
    gfxCachePresent(gfx, "main");
    gfxRefresh(gfx);
    gfxPollEvents(gfx);
  }
  free(help_screen_ascii);
  return 0;
}

GameAction gfxUserInput(Gfx gfx, int entity_index)
{
  gfxPollEvents(gfx);
  if(getExitState() == 1) return noAction();

  uint32_t input_code = gfxInputUnicode();
  if(input_code == 0) return noAction();
  
  /*
  static double old_time = 0;
  double delta_time = glfwGetTime() - old_time;
  char fps_str[1024];
  snprintf ( fps_str, 1024, "%f", 1.0f / delta_time );
  gfxDrawString(fps_str, 0, 0, 15, 0);
  old_time = glfwGetTime();
  */

  GameAction action = noAction();
  switch(input_code) {
  case 's':
    action = moveEntityAction(entity_index, 0, 1);
    break;
  case 'w':
    action = moveEntityAction(entity_index, 0, -1);
    break;
  case 'a':
    action = moveEntityAction(entity_index, -1, 0);
    break;
  case 'd':
    action = moveEntityAction(entity_index, 1, 0);
    break;
  case 'b':
    action = guiPickTile(gfx);
    break;
  case '.':
    action = dropAction(entity_index);
    break;
  case '?':
    userInputHelpScreen(gfx);
    break;
  }
   
  if(input_code >= PUA_START){
    int mouse_x, mouse_y;
    gfxMousePos(&mouse_x, &mouse_y);
    int center_x = ASCII_SCREEN_WIDTH / 2;
    int center_y = ASCII_SCREEN_HEIGHT / 2;
    int delta_x = 0, delta_y = 0;
    if(mouse_x > center_x) delta_x = 1;
    if(mouse_x < center_x) delta_x = -1;
    if(mouse_y > center_y) delta_y = 1;
    if(mouse_y < center_y) delta_y = -1;
    action = moveEntityAction(entity_index, delta_x, delta_y);
  } // end of mouse input
 
  return action;
}
