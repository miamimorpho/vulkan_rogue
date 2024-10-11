#include "vulkan_meta.h"

int inputInit(void);
int gfxConstInit(void);
int gfxGlobalInit(void);

int gfxTextureLoad(const char*);
GfxTileset gfxGetTexture(int);

int gfxDrawStart(void);
int gfxBlitImage(int);
int gfxDrawChar(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
int gfxDrawString(const char*, uint32_t, uint32_t, uint32_t, uint32_t);
int gfxDrawGradient(vec2, vec2, uint32_t, uint32_t);

int gfxDrawEnd(void);
int gfxClose(void);
