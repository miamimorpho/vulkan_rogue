#include "vulkan_meta.h"

ivec3 hexColor(uint32_t);
int _gfxDrawStart(GfxConst, GfxGlobal*);
int _gfxBlitImage(GfxConst, GfxGlobal*, int);
int _gfxDrawChar(GfxConst, GfxGlobal*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
int _gfxDrawString(GfxConst, GfxGlobal*, const char*, int, int, int);
int _gfxDrawTileset(GfxConst, GfxGlobal*, int);
int _gfxDrawEnd(GfxConst, GfxGlobal*);
