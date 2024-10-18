#include "vulkan_meta.h"

int inputInit(void);
int gfxScreenInit(void);

int gfxTextureLoad(const char*);
GfxTileset gfxGetTexture(int);

int gfxRefresh(void);
int gfxAddCh(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
int gfxDrawString(const char*, uint32_t, uint32_t, uint32_t, uint32_t);

int gfxScreenClose(void);
