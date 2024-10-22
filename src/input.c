#include "vulkan_public.h"
#include "action.h"
#include <stdio.h>
#include <stdlib.h>

#define GLFW_MOUSE_UNKNOWN -1

typedef struct{
  int key;
  int mods;
  
  int mouse_x;
  int mouse_y;
  int mouse_btn;
  
  int pressed;
  int to_exit;
  double time;
} inputState_t;

static inputState_t s_state;

int getInputState(inputState_t* dst)
{
  glfwPollEvents();
  GfxContext gfx = gfxGetContext();

  // convert screen pos to tile pos
  double xpos, ypos;
  glfwGetCursorPos(gfx.window, &xpos, &ypos);
  s_state.mouse_x = (int)xpos / (ASCII_SCALE * ASCII_TILE_SIZE);
  s_state.mouse_y = (int)ypos / (ASCII_SCALE * ASCII_TILE_SIZE);

  // send duplicate, reset values
  inputState_t src_cpy = s_state;  
  s_state.key = GLFW_KEY_UNKNOWN;
  s_state.mouse_btn = GLFW_MOUSE_UNKNOWN; 
  s_state.pressed = 0;

  if(dst != NULL){
    *dst = src_cpy;
    if(dst->pressed == 1){
      return 1;
    }
  }
  return 0;
}

int getExitState(void){
  return s_state.to_exit;
}

void closeCallback(GLFWwindow* window) {
    s_state.to_exit = 1;
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (action == GLFW_PRESS){
    s_state.pressed = 1;
    s_state.mouse_btn = button;
  }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(glfwGetTime() - s_state.time < 0.06){
    s_state.key = GLFW_KEY_UNKNOWN;
    s_state.pressed = 0;
    return;
  }
  s_state.time = glfwGetTime();

  s_state.key = key;
  s_state.mods = mods;
  
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    s_state.pressed = 1;
  }
  if(action == GLFW_RELEASE){
    s_state.pressed = 0;
  }

}

void _inputInit(GfxContext gfx){
  glfwMakeContextCurrent(gfx.window);
  glfwSetInputMode(gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwSetCursorPos(gfx.window, 0,0);

  glfwSetWindowCloseCallback(gfx.window, closeCallback);
  glfwSetKeyCallback(gfx.window, keyCallback);
  glfwSetMouseButtonCallback(gfx.window, mouseCallback);
  
  getInputState(NULL);
  s_state.time = 0;
}

// Game specific input code

GameAction guiPickColor(void)
{
  uint32_t fgIndex = -1;
  uint32_t bgIndex = -1;
  inputState_t input;
  while(getInputState(&input) == 0){
 
    fgIndex = input.mouse_x;
    bgIndex = input.mouse_y;
    
    for(int i = 0; i < PALETTE_SIZE * PALETTE_SIZE; i++){
      gfxAddCh(i % PALETTE_SIZE,
	       i / PALETTE_SIZE,
	       939,
	       i % PALETTE_SIZE,
	       i / PALETTE_SIZE,
	       DRAW_TEXTURE_INDEX);
    }
    gfxRefresh();
  } // End of UI Loop
  return paintEntityAction(-1,fgIndex, bgIndex);
}

GameAction guiPickTile(void)
{
  GfxTileset tileset = gfxGetTexture(DRAW_TEXTURE_INDEX);

  int width_in_tiles =
    (int)(tileset.image_w) / tileset.glyph_w;
  // some tilesets are a long strip and need resizing, others benefit from  being displayed in the dimensions they were made in
  if(width_in_tiles == 1){
    width_in_tiles = ASCII_SCREEN_WIDTH;
  }
  
  uint32_t target_uv = 0;
  inputState_t input;
  while(getInputState(&input) == 0){

    target_uv = (input.mouse_y * width_in_tiles) + input.mouse_x;
 
    for(uint32_t i = 0; i < tileset.glyph_c; i++){
      int x = i % width_in_tiles;
      int y = i / width_in_tiles;
      
      uint32_t bg = 0;
      if((x % 2) - (y % 2) == 0) bg = 1;
      
      gfxAddCh(x, y, i,
	       15,
	       bg,
	       DRAW_TEXTURE_INDEX );
    }
    
    gfxAddCh(input.mouse_x, input.mouse_y, target_uv,
	     11, 0,
	     DRAW_TEXTURE_INDEX);
    
    gfxRefresh();
    
  } // end of UI loop
  return paintEntityAction(target_uv, -1, -1);
}

int userInputHelpScreen(void)
{
  char* help_screen_ascii = configReadFile("data/controls.txt");

  inputState_t input;
  while(getInputState(&input) == 0){
    gfxDrawString(help_screen_ascii, 0, 0, 15, 0);
    gfxRefresh();
  }
  free(help_screen_ascii);
  return 0;
}

GameAction userInput(int entity_index)
{
  inputState_t input;
  getInputState(&input); 
  if (input.to_exit == 1) {
    return noAction();
  }
  if(input.pressed == 0){
    return noAction();
  }
  
  /*
  static double old_time = 0;
  double delta_time = glfwGetTime() - old_time;
  char fps_str[1024];
  snprintf ( fps_str, 1024, "%f", 1.0f / delta_time );
  gfxDrawString(fps_str, 0, 0, 15, 0);
  old_time = glfwGetTime();
  */

  GameAction action = noAction();
  // First Keyboard Input
  switch(input.key) {
  case GLFW_KEY_UNKNOWN:
    break;
  case GLFW_KEY_S:
    action = moveEntityAction(entity_index, 0, 1);
    break;
  case GLFW_KEY_W:
    action = moveEntityAction(entity_index, 0, -1);
    break;
  case GLFW_KEY_A:
    action = moveEntityAction(entity_index, -1, 0);
    break;
  case GLFW_KEY_D:
    action = moveEntityAction(entity_index, 1, 0);
    break;
  case GLFW_KEY_B:
    action = guiPickTile();
    break;
  case GLFW_KEY_C:
    action = guiPickColor();
    break;
  case GLFW_KEY_PERIOD:
    action = dropAction(entity_index);
    break;
  }
  
  // Shift Keyboard Input
  if (input.mods & GLFW_MOD_SHIFT){
    switch(input.key) {
    case GLFW_KEY_SLASH: // Question Mark ?
      userInputHelpScreen();
      break;
    }
  }  
  
  if(input.mouse_btn != GLFW_MOUSE_UNKNOWN){
    int center_x = ASCII_SCREEN_WIDTH / 2;
    int center_y = ASCII_SCREEN_HEIGHT / 2;
    int delta_x = 0, delta_y = 0;
    if(input.mouse_x > center_x) delta_x = 1;
    if(input.mouse_x < center_x) delta_x = -1;
    if(input.mouse_y > center_y) delta_y = 1;
    if(input.mouse_y < center_y) delta_y = -1;
    action = moveEntityAction(entity_index, delta_x, delta_y);
  } // end of mouse input
 
  return action;
}
