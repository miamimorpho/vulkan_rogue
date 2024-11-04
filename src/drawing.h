#include "vulkan_meta.h"

int gfxPipelineInit(GfxContext*);
void gfxClear(GfxGlobal* dst );
int gfxAddCh(GfxGlobal*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
int gfxAddString(GfxGlobal*, uint16_t, uint16_t, const char*, uint16_t, uint16_t);

int gfxCacheChange(GfxGlobal*, const char*);
int gfxCachePresent(GfxGlobal*, const char*);
int gfxRefresh(GfxGlobal*);
