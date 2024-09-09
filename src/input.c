#include "vulkan_helper.h"
#include "action.h"
#include <time.h>

typedef struct{
  int key;
  int pressed;
  int to_exit;
} inputState_t;

static inputState_t s_state;

void closeCallback(GLFWwindow* window) {
    printf("Close callback triggered!\n");
    s_state.to_exit = 1;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS) {
    s_state.key = key;
    s_state.pressed = 1;
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
  s_state.pressed = 1;
  s_state.to_exit = 0;
}

GameAction userInput(void){
  
  glfwPollEvents();

  if (s_state.to_exit == 1) {
    return requestAction(noAction, 0, NULL);
  }
  
  if(s_state.pressed == 1){

    Pos v = { 0, 0};
    
    switch(s_state.key) {
    case GLFW_KEY_S:
      v.x = 0;
      v.y = 1;
      break;
    case GLFW_KEY_W:
      v.x = 0;
      v.y = -1;
      break;
    case GLFW_KEY_A:
      v.x = -1;
      v.y = 0;
      break;
    case GLFW_KEY_D:
      v.x = 1;
      v.y = 0;
      break;
    }
    GameAction action = requestAction
      (moveEntityAction, 3,
       ARG_INT, 0,
       ARG_INT, v.x,
       ARG_INT, v.y);
    
    s_state.pressed = 0;
    s_state.key = GLFW_KEY_UNKNOWN;

    return action;
  }

  return requestAction(noAction, 0, NULL);
}
