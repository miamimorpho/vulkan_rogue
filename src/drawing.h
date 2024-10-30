#include "vulkan_meta.h"

int gfxPipelineInit(GfxContext*);
int _gfxBakeCommandBuffers(GfxContext, int, VkBuffer, int);

int _gfxRenderFrame(GfxContext, GfxGlobal*);

int _gfxAddString(GfxContext, GfxGlobal*, uint16_t, uint16_t, const char*, uint16_t, uint16_t);
int _gfxAddCh(GfxGlobal*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);

int cacheCreate(TileDrawCache*, const char*, enum TileDrawCacheType);
int _gfxCacheChange(GfxGlobal*, const char*);
int _gfxDrawCache(GfxContext, GfxGlobal*, const char*);
