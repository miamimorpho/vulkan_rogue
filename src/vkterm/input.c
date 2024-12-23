#include "vkterm_private.h"
#include "input.h"
#include <stdio.h>

typedef struct{
  double time;
  uint32_t unicode;
  double mouse_x_norm;
  double mouse_y_norm;
  uint8_t exit_status;
} GfxInput;

static GfxInput g_input;

uint32_t gfxGetKey(void){
  return g_input.unicode;
}

void characterCallback(GLFWwindow* window, unsigned int codepoint)
{
  g_input.time = glfwGetTime();
  g_input.unicode = codepoint;
}

void gfxPollEvents(GfxGlobal* gfx){

  static double old_time = 0;

  const double press_delay = (1/30);
  
  g_input.unicode = 0;
  glfwWaitEventsTimeout(1);
  if(g_input.time - old_time < press_delay){
    g_input.unicode = 0;
  }else{
    old_time = g_input.time;
  }
  
  double xpos, ypos;
  glfwGetCursorPos(gfx->vk.window, &xpos, &ypos);
 
  g_input.mouse_x_norm = xpos / gfx->vk.extent.width;
  g_input.mouse_y_norm = ypos / gfx->vk.extent.height;
  
}

int getExitState(void){
  if(g_input.exit_status == 1) return 1;
  return 0;
}

void closeCallback(GLFWwindow* window) {
    g_input.exit_status = 1;
}

double gfxMouseX(void){
    return g_input.mouse_x_norm;
}

double gfxMouseY(void){
    return g_input.mouse_y_norm;
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (action == GLFW_PRESS){
    g_input.time = glfwGetTime();
    g_input.unicode = PUA_START + button;
  }
}

void gfxInputInit(GLFWwindow* window){
  glfwMakeContextCurrent(window);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwSetCursorPos(window, 0,0);

  glfwSetWindowCloseCallback(window, closeCallback);
  glfwSetCharCallback(window, characterCallback);
  //glfwSetKeyCallback(window, keyCallback);
  glfwSetMouseButtonCallback(window, mouseCallback);

  g_input.exit_status = 0;
  g_input.time = 0;
}
