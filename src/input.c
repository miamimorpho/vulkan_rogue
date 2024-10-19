//#include "vulkan_helper.h"
#include "vulkan_public.h"
#include "action.h"
#include <stdio.h>

#define GLFW_MOUSE_UNKNOWN -1

typedef struct{
  int key;
  double mouse_x;
  double mouse_y;
  int mouse_btn;
  int pressed;
  int to_exit;
  double time;
} inputState_t;

static inputState_t s_state;

void resetInputState(void){
  s_state.key = GLFW_KEY_UNKNOWN;
  s_state.mouse_btn = GLFW_MOUSE_UNKNOWN; 
  s_state.pressed = 0;
  s_state.to_exit = 0;

}

void closeCallback(GLFWwindow* window) {
    printf("Close callback triggered!\n");
    s_state.to_exit = 1;
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
  GfxContext gfx = gfxGetContext();
  glfwGetCursorPos(gfx.window, &s_state.mouse_x, &s_state.mouse_y);
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

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    s_state.key = key;
    s_state.pressed = 1;
    s_state.time = glfwGetTime();
  }
  if(action == GLFW_RELEASE){
    s_state.key = key;
    s_state.pressed = 0;
    s_state.time = glfwGetTime();
  }

}

int getExitState(void){
  return s_state.to_exit;
}

void _inputInit(GfxContext gfx){
  glfwMakeContextCurrent(gfx.window);
  glfwSetInputMode(gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwSetCursorPos(gfx.window, 0,0);

  glfwSetWindowCloseCallback(gfx.window, closeCallback);
  glfwSetKeyCallback(gfx.window, keyCallback);
  glfwSetMouseButtonCallback(gfx.window, mouseCallback);
  
  resetInputState();
  s_state.time = 0;
}

inputState_t getInputState(void){
  glfwPollEvents();
  return s_state;
}

GameAction guiPickColor(void){
  resetInputState();
  int to_exit = 0;

  uint32_t hex_color = 0xFFFFFFFF;

  uint32_t fgIndex = -1;
  uint32_t bgIndex = -1;
  while(to_exit == 0){
    gfxDrawString("color", 0, 1, hex_color, 0xFF000000);

    for(int i = 0; i < PALETTE_SIZE * PALETTE_SIZE; i++){
      gfxAddCh(i % PALETTE_SIZE,
	       i / PALETTE_SIZE,
	       939,
	       i % PALETTE_SIZE,
	       i / PALETTE_SIZE,
	       DRAW_TEXTURE_INDEX);
    }

    gfxRefresh();
    
    inputState_t inputState = getInputState();
    
    fgIndex = inputState.mouse_x / (ASCII_TILE_SIZE * ASCII_SCALE);
    bgIndex = inputState.mouse_y / (ASCII_TILE_SIZE * ASCII_SCALE);
    
    if(inputState.pressed == 1){
      to_exit = 1;
    }
  }
  return paintEntityAction(-1, fgIndex, bgIndex);
}

GameAction guiPickTile(void){
  resetInputState();

  GfxContext gfx = gfxGetContext();
  GfxTileset tileset = gfxGetTexture(DRAW_TEXTURE_INDEX);
  int width_in_tiles = (int)(gfx.extent.width / (tileset.glyph_width * ASCII_SCALE));

  uint32_t target_uv = 0;
  int to_exit = 0;
  while(to_exit == 0){
    
   inputState_t input = getInputState();
    
   uint32_t target_x = (int)(input.mouse_x / (ASCII_SCALE * tileset.glyph_width));
   uint32_t target_y = (int)(input.mouse_y / (ASCII_SCALE * tileset.glyph_height));
   target_uv = (target_y * width_in_tiles) + target_x;

   if(input.pressed == 1){
     printf("uv %d\n", target_uv);
     to_exit = 1;
   }
   
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

   gfxAddCh(target_x, target_y, target_uv,
	    11, 0,
	    DRAW_TEXTURE_INDEX);
   
   gfxRefresh();
   
  }

  return paintEntityAction(target_uv, -1, -1);
}

GameAction userInput(int entity_index){

  GfxContext gfx = gfxGetContext();
  
  glfwPollEvents();
  if (s_state.to_exit == 1) {
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
  
  // Start of keyboard input
  if(s_state.pressed == 1){
    GameAction action = noAction();

    // keyboard input
    if(s_state.key != GLFW_KEY_UNKNOWN){
      switch(s_state.key) {
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
      resetInputState();
      return action;
    } // end of keyboard input

    if(s_state.mouse_btn != GLFW_MOUSE_UNKNOWN){
      int texel_size = ASCII_SCALE * ASCII_TILE_SIZE;
      double center_x = ASCII_SCREEN_WIDTH * texel_size / 2;
      double center_y = ASCII_SCREEN_HEIGHT * texel_size / 2;
      double xpos, ypos;
      glfwGetCursorPos(gfx.window, &xpos, &ypos);
      int delta_x = 0, delta_y = 0;
      int tol = texel_size * 3;
      if(xpos > center_x + tol) delta_x = 1;
      if(xpos < center_x - tol) delta_x = -1;
      if(ypos > center_y + tol) delta_y = 1;
      if(ypos < center_y - tol) delta_y = -1;
      action = moveEntityAction(entity_index, delta_x, delta_y);
      resetInputState();
      return action;
    }
  
  }

  return noAction();
}
