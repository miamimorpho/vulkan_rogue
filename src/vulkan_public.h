#include "vulkan_meta.h"

int inputInit(void);
int gfxConstInit(void);
int gfxGlobalInit(void);

int gfxTextureLoad(const char*);
GfxTileset gfxGetTexture(int);

int gfxDrawStart(void);
int gfxBlitImage(int);
int gfxDrawChar(uint32_t, int, int, int, int);
int gfxDrawString(const char*, int, int, int);

int gfxDrawEnd(void);
int gfxClose(void);
