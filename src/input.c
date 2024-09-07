#include "vulkan_helper.h"
#include "world.h"
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

int userInput(void){
  
  glfwPollEvents();

  if (s_state.to_exit == 1) {
    return 1;
  }
  
  if(s_state.pressed == 1){

    actionArgs = {
      .entity_id = 0;
      .move_x = 0;
      .move_y = 0;
    }
    
    switch(s_state.key) {
    case GLFW_KEY_S:
      entityMove(player, 0, 1);
      break;
    case GLFW_KEY_W:
      entityMove(player, 0, -1);
      break;
    case GLFW_KEY_A:
      entityMove(player, -1, 0);
      break;
    case GLFW_KEY_D:
      entityMove(player, 1, 0);
      break;
    }
    s_state.pressed = 0;
    s_state.key = GLFW_KEY_UNKNOWN;
    action_t action{
      .func = moveEntity;
      .args = [1, 1];
    return 
  }

  return 0;
}
