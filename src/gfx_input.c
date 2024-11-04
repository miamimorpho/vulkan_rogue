#include "vulkan_meta.h"
#include <stdio.h>

/*
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
*/

typedef struct{
  double time;
  uint32_t unicode;
  uint16_t mouse_x;
  uint16_t mouse_y;
  int to_exit;
} GfxInputState;

static GfxInputState s_state;

uint32_t gfxInputUnicode(void){
  return s_state.unicode;
}

void characterCallback(GLFWwindow* window, unsigned int codepoint)
{
  s_state.time = glfwGetTime();
  s_state.unicode = codepoint;
}

void gfxPollEvents(GfxGlobal* gfx){

  static double old_time = 0;

  const double press_delay = (1/30);
  
  s_state.unicode = 0;
  glfwWaitEventsTimeout(1);
  if(s_state.time - old_time < press_delay){
    s_state.unicode = 0;
  }else{
    old_time = s_state.time;
  }
  
  double xpos, ypos;
  glfwGetCursorPos(gfx->vk.window, &xpos, &ypos);
  s_state.mouse_x = (int)((int)xpos / (ASCII_SCALE * ASCII_TILE_SIZE));
  s_state.mouse_y = (int)((int)ypos / (ASCII_SCALE * ASCII_TILE_SIZE));

}

/*
int getInputState(GLFWwindow* window, inputState_t* dst)
{
  // excessive input polling kills battery-life
  glfwWaitEventsTimeout(0.7);

  // convert screen pos to tile pos
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  s_state.mouse_x = (int)((int)xpos / (ASCII_SCALE * ASCII_TILE_SIZE));
  s_state.mouse_y = (int)((int)ypos / (ASCII_SCALE * ASCII_TILE_SIZE));

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
*/

int getExitState(void){
  return s_state.to_exit;
}

void closeCallback(GLFWwindow* window) {
    s_state.to_exit = 1;
}

void gfxMousePos(int* dst_x, int* dst_y){
  *dst_x = s_state.mouse_x;
  *dst_y = s_state.mouse_y;
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (action == GLFW_PRESS){
    s_state.time = glfwGetTime();
    s_state.unicode = PUA_START + button;
  }
}

/*
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
*/

void gfxInputInit(GLFWwindow* window){
  glfwMakeContextCurrent(window);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwSetCursorPos(window, 0,0);

  glfwSetWindowCloseCallback(window, closeCallback);
  glfwSetCharCallback(window, characterCallback);
  //glfwSetKeyCallback(window, keyCallback);
  glfwSetMouseButtonCallback(window, mouseCallback);
  
  s_state.time = 0;
}
