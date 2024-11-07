#include <GLFW/glfw3.h>

#define PUA_START 0xE000 // 57343
/* provides 6400 chars */
#define PUA_LEFT_CLICK (PUA_START + GLFW_MOUSE_BUTTON_LEFT)

enum GfxInputState{
  HOVER,
  QUIT,
  OUT_OF_BOUNDS
};

typedef struct{
  double time;
  uint32_t unicode;
  uint16_t mouse_x;
  uint16_t mouse_y;
  enum GfxInputState state;
} GfxInput;

GfxInput gfxGetInput(void);
