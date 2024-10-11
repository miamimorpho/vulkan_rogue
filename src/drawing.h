#include "vulkan_meta.h"

ivec3 hexColor(uint32_t);
int _gfxDrawStart(GfxContext, GfxGlobal*);
int _gfxBlitImage(GfxContext, GfxGlobal*, int);
int _gfxDrawChar(GfxContext, GfxGlobal*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
int _gfxDrawString(GfxContext, GfxGlobal*, const char*, uint32_t, uint32_t, uint32_t, uint32_t);
int _gfxDrawGradient(GfxContext, GfxGlobal*, vec2, vec2, uint32_t, uint32_t);
int _gfxDrawTileset(GfxContext, GfxGlobal*, int);
int _gfxDrawEnd(GfxContext, GfxGlobal*);
