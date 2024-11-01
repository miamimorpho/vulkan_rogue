#include "vulkan_meta.h"

int gfxPipelineInit(GfxContext*);

int _gfxAddCh(GfxGlobal*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
int _gfxAddString(GfxContext, GfxGlobal*, uint16_t, uint16_t, const char*, uint16_t, uint16_t);

int _gfxCacheChange(GfxContext, GfxGlobal*, const char*);
int _gfxCachePresent(GfxContext, GfxGlobal*, const char*);
int _gfxRefresh(GfxContext, GfxGlobal*);
