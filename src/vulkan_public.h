#include "maths.h"

int inputInit(void);
int gfxConstInit(void);
int gfxGlobalInit(void);
int gfxTilesetLoad(char* filename);
int gfxDrawStart(void);
int gfxDrawChar(char, int, int, ivec3);
int gfxDrawEnd(void);
int gfxClose(void);
