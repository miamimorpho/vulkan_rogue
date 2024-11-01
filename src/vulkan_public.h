#include "vulkan_meta.h"

int inputInit(void);
int gfxScreenInit(void);

int gfxTextureLoad(const char*);
GfxTileset gfxGetTexture(int);

int gfxRefresh(void);
int gfxCacheChange(const char*);
int gfxCachePresent(const char*);
int gfxAddCh(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
int gfxAddString(uint16_t, uint16_t, const char*, uint16_t, uint16_t);

int gfxScreenClose(void);
