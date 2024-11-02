#include "vulkan_meta.h"

int gfxScreenInit(GfxGlobal*);
int gfxTextureLoad(GfxGlobal*, const char*);
int gfxAddCh(GfxGlobal*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
int gfxAddString(GfxGlobal*, uint16_t, uint16_t, const char*, uint16_t, uint16_t);
int gfxCacheChange(GfxGlobal*, const char*);
int gfxCachePresent(GfxGlobal*, const char*);
int gfxRefresh(GfxGlobal*);
int gfxScreenClose(GfxGlobal);
