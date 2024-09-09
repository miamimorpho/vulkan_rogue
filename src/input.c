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
    return noAction();
  }
  
  if(s_state.pressed == 1){

    int x, y = 0;
    
    switch(s_state.key) {
    case GLFW_KEY_S:
      x = 0;
      y = 1;
      break;
    case GLFW_KEY_W:
      x = 0;
      y = -1;
      break;
    case GLFW_KEY_A:
      x = -1;
      y = 0;
      break;
    case GLFW_KEY_D:
      x = 1;
      y = 0;
      break;
    }
    GameAction action = moveEntityAction(0, x, y);
    
    s_state.pressed = 0;
    s_state.key = GLFW_KEY_UNKNOWN;

    return action;
  }

  return noAction();
}
