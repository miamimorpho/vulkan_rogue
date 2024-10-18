#include "vulkan_meta.h"

int _gfxRefresh(GfxContext, GfxGlobal*);
int _gfxDrawCharRaw(GfxContext, GfxGlobal*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
int _gfxDrawString(GfxContext, GfxGlobal*, const char*, uint32_t, uint32_t, uint32_t, uint32_t);
int _gfxAddCh(GfxGlobal*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
