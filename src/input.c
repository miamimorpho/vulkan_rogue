#include "vulkan_helper.h"
#include "action.h"
#include <stdio.h>

typedef struct{
  int key;
  int pressed;
  int to_exit;
  double time;
} inputState_t;

static inputState_t s_state;

void closeCallback(GLFWwindow* window) {
    printf("Close callback triggered!\n");
    s_state.to_exit = 1;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
  
  s_state.key = GLFW_KEY_UNKNOWN;
  s_state.pressed = 0;
  s_state.to_exit = 0;
  s_state.time = 0;
}

GameAction userInput(int entity_index){

  static double last_key_time = 0;
  
  glfwPollEvents();
  if (s_state.to_exit == 1) {
    return noAction();
  }

  if(s_state.time - last_key_time > 0.06 && s_state.pressed == 1){
    last_key_time = s_state.time;
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
    case GLFW_KEY_PERIOD:
      action = buildWallAction(entity_index);
      break;
    }
    s_state.pressed = 0;
    s_state.key = GLFW_KEY_UNKNOWN;
    
    return action;
  }

  return noAction();
}
