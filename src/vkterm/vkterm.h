#ifndef VKTERM_PUBLIC_H
#define VKTERM_PUBLIC_H
//#include "config.h"
#include "input.h"
#include <stdint.h>

struct GfxGlobal;
typedef struct GfxGlobal* Gfx;

/* Init + Settings */
Gfx gfxScreenInit(void);
int gfxTextureLoad(Gfx, const char*);
int gfxScreenClose(Gfx);
int gfxGetScreenWidth(Gfx);
int gfxGetScreenHeight(Gfx);

/* Drawing Functions */
void gfxClear(Gfx);
int gfxRenderGlyph(Gfx, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
int gfxRenderElement(Gfx, uint16_t, uint16_t, const char*, uint16_t, uint16_t, uint16_t);
int gfxCacheChange(Gfx, const char*);
int gfxCachePresent(Gfx, const char*);
int gfxRefresh(Gfx);

/* Input Functions */
void gfxPollEvents(Gfx);
double gfxGetMouseX(void);
double gfxGetMouseY(void);
uint32_t gfxGetKey(void);
int getExitState(void);

#endif // VKTERM_PUBLIC_H
