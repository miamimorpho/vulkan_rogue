
#include "controls.h"
#include "action.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GfxInput guiPad(Gfx gfx, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
  GfxInput input = gfxGetInput();
  if(input.mouse_x >= x1 && input.mouse_x < x2 &&
     input.mouse_y >= y1 && input.mouse_y < y2){
        gfxAddCh(gfx, input.mouse_x, input.mouse_y, 481, DRAW_TEXTURE_INDEX, 3, 0);
	input.mouse_x -= x1;
	input.mouse_y -= y1;
  }else{
    input.mouse_x = 0;
    input.mouse_y = 0;
    input.state = OUT_OF_BOUNDS;
    input.unicode = 0;
  }  
  return input;
}

int guiButton(Gfx gfx, uint16_t x, uint16_t y, const char* label, uint16_t fg, uint16_t bg){
  gfxAddString(gfx, x, y, label, fg, bg);
  int mouse_x, mouse_y;
  gfxMousePos(&mouse_x, &mouse_y);
  if(mouse_x >= x && mouse_x < x + strlen(label) &&
     mouse_y == y && gfxInputUnicode() == PUA_LEFT_CLICK){
    return 1;
  }
  return 0;
}

GameAction guiPickTile(Gfx gfx)
{
 
  const int viewport_size = 16;
  
  gfxCacheChange(gfx, "background");

  // foreground color picker
  const int fg_pick_offset = 1;
  gfxAddString(gfx, 0, fg_pick_offset -1, "Foreground", 15, 0);
  for(int i = 0; i < viewport_size; i++){
    gfxAddCh(gfx, i, fg_pick_offset, 0, DRAW_TEXTURE_INDEX,
	     i, i);
  }
  // background color picker
  const int bg_pick_offset = 3;
  gfxAddString(gfx, 0, bg_pick_offset - 1, "Background", 15, 0);
  for(int i = 0; i < viewport_size; i++){
    gfxAddCh(gfx, i, bg_pick_offset, 0, DRAW_TEXTURE_INDEX,
	     i, i);
  }

  static uint32_t atlas_page = 0;
  static uint32_t target_uv = 0;
  static uint32_t target_fg = 15;
  static uint32_t target_bg = 0;
  
  // GUI Render Loop
  gfxCacheChange(gfx, "main");
  while(1){
    gfxClear(gfx);

    int mouse_x, mouse_y;
    gfxMousePos(&mouse_x, &mouse_y);

    GfxInput fg_pick = guiPad(gfx, 0, fg_pick_offset, 16, fg_pick_offset +1);
    if(fg_pick.unicode == PUA_LEFT_CLICK){
      target_fg = fg_pick.mouse_x;
    }

    GfxInput bg_pick = guiPad(gfx, 0, bg_pick_offset, 16, bg_pick_offset +1);
    if(bg_pick.unicode == PUA_LEFT_CLICK){
      target_bg = bg_pick.mouse_x;
    }

    // glyph picker
    int page_x_offset = (atlas_page % 2) * viewport_size;
    int page_y_offset = (atlas_page / 2) * viewport_size;
    
    const int glyph_pick_offset = 5;  
    gfxAddString(gfx, 0, glyph_pick_offset -1, "Glyph", 15, 0);
    for(int y = 0; y < viewport_size; y++){
      for(int x = 0; x < viewport_size; x++){
	int i = ((y + page_y_offset) * ATLAS_WIDTH) + x + page_x_offset;
	uint32_t bg = 0;
	if((x % 2) - (y % 2) == 0) bg = 1;
	gfxAddCh(gfx, x,
		 y + glyph_pick_offset, i, DRAW_TEXTURE_INDEX,
		 15, bg);
      }// end of x loop
    }// end of y loop
    
    GfxInput glyph_pick = guiPad(gfx, 0, glyph_pick_offset, 16, glyph_pick_offset +16);
    if(glyph_pick.unicode == PUA_LEFT_CLICK){
      target_uv = ((glyph_pick.mouse_y + page_y_offset) * ATLAS_WIDTH) + glyph_pick.mouse_x + page_x_offset;
      //target_uv = glyph_pick.mouse_y * ATLAS_WIDTH + glyph_pick.mouse_x + page_offset;
      printf("%d\n", target_uv);
    }

    if(guiButton(gfx, 13, 21, "<", 0, 8) == 1){
      atlas_page = (atlas_page - 1) % 4;
    }
    if(guiButton(gfx, 15, 21, ">", 0, 8) == 1){
      atlas_page = (atlas_page + 1) % 4;
    }
    
    if(guiButton(gfx, 0, 22, " Done ", 0, 8) == 1){
      break;
    }
    
    gfxCachePresent(gfx, "background");
    gfxCachePresent(gfx, "main");
    gfxRefresh(gfx);
    gfxPollEvents(gfx);
  } // end of GUI loop
  return paintEntityAction(target_uv, target_fg, target_bg);
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

GameAction gfxUserInput(Gfx gfx)
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
    action = moveMobileAction(0, 1);
    break;
  case 'w':
    action = moveMobileAction(0, -1);
    break;
  case 'a':
    action = moveMobileAction(-1, 0);
    break;
  case 'd':
    action = moveMobileAction(1, 0);
    break;
  case 'b':
    action = guiPickTile(gfx);
    break;
  case '.':
    action = buildTerrainAction(0);
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
    action = moveMobileAction(delta_x, delta_y);
  } // end of mouse input
 
  return action;
}
