#include "vulkan_helper.h"
#include "vulkan_public.h"
#include "action.h"
#include <stdio.h>

typedef struct{
  int key;
  int mouse_btn;
  int pressed;
  int to_exit;
  double time;
} inputState_t;

static inputState_t s_state;

void resetInputState(void){
  s_state.key = GLFW_KEY_UNKNOWN;
  s_state.mouse_btn = -1; 
  s_state.pressed = 0;
  s_state.to_exit = 0;

}

void closeCallback(GLFWwindow* window) {
    printf("Close callback triggered!\n");
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

void _inputInit(GfxConst gfx){
  glfwMakeContextCurrent(gfx.window);
  glfwSetInputMode(gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwSetCursorPos(gfx.window, 0,0);

  glfwSetWindowCloseCallback(gfx.window, closeCallback);
  glfwSetKeyCallback(gfx.window, keyCallback);
  glfwSetMouseButtonCallback(gfx.window, mouseCallback);
  
  resetInputState();
  s_state.time = 0;
}

GameAction guiPickTile(void){
  resetInputState();

  GfxConst gfx = gfxGetConst();
  GfxTileset tileset = gfxGetTexture(1);
  int width_in_tiles = tileset.width / tileset.glyph_width;

  uint32_t target_uv = 0;
  int to_exit = 0;
  while(to_exit == 0){

    gfxDrawStart();

    for(uint i = 0; i < tileset.glyph_c; i++){
      int x = i % width_in_tiles;
      int y = i / width_in_tiles;
      gfxDrawChar(getUnicodeUV(tileset, i), x, y, 0xFFFFFF, 1 );
    }

    gfxDrawEnd();
    double xpos, ypos;
    glfwPollEvents();
    if(s_state.pressed == 1){
      glfwGetCursorPos(gfx.window, &xpos, &ypos);
      uint32_t target_x = xpos / (ASCII_SCALE * tileset.glyph_width);
      uint32_t target_y = ypos / (ASCII_SCALE * tileset.glyph_height);
      target_uv = (target_y * width_in_tiles) + target_x;
      printf("uv %d", target_uv);
      to_exit = 1;
    }
  
  }

  return pickTileAction(target_uv, 0);
}

GameAction userInput(int entity_index){

  glfwPollEvents();
  if (s_state.to_exit == 1) {
    return noAction();
  }

  if(s_state.pressed == 1){
    GameAction action = noAction();
    
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
    case GLFW_KEY_PERIOD:
      action = dropAction(entity_index);
      break;
    }
    resetInputState();
    return action;
  }

  return noAction();
}
