#include "vulkan_meta.h"

int inputInit(void);
int gfxScreenInit(void);

int gfxTextureLoad(const char*);
GfxTileset gfxGetTexture(int);

int gfxRenderFrame(void);
int gfxPresentFrame(void);
int gfxAddCh(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
int gfxAddString(uint16_t, uint16_t, const char*, uint16_t, uint16_t);

int gfxScreenClose(void);
