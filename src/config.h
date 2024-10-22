/* Asset Loading Options */
#define COLOR_TEXTURE_INDEX 0
#define PALETTE_SIZE 16
#define ASCII_TEXTURE_INDEX 1
#define DRAW_TEXTURE_INDEX 2

/* ASCII Options */
#define ASCII_SCREEN_WIDTH 32
#define ASCII_SCREEN_HEIGHT 24
#define ASCII_TILE_SIZE 8
#define ASCII_SCALE 2.0f

/* Renderer Options */
/* Tested on ThinkpadX230 */
#define DRAW_BUFFER_COUNT 2
#define MAX_SAMPLERS 64

/* Build Options */
#define DEBUG_BUFFER 0
#define DEBUG_VALIDATION 1

char* configReadFile(const char*);
