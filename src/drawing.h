#include "vulkan_meta.h"

int _gfxRenderFrame(GfxContext, GfxGlobal*);
int _gfxPresentFrame(GfxContext, GfxGlobal*);
int _gfxAddString(GfxContext, GfxGlobal*, uint16_t, uint16_t, const char*, uint16_t, uint16_t);
int _gfxAddCh(GfxGlobal*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
int gfxPipelineInit(GfxContext*);
