#ifndef VKTERM_PUBLIC_H
#define VKTERM_PUBLIC_H
#include "input.h"
#include <stdint.h>

struct GfxGlobal;
typedef struct GfxGlobal* Gfx;

/* Init + Settings 
 * vkterm_private.c */
Gfx gfxScreenInit(void);
int gfxScreenClose(Gfx);
int gfxGetScreenWidth(Gfx);
int gfxGetScreenHeight(Gfx);

/* Textures.h */
int gfxTextureLoad(Gfx, const char*);

/* Drawing Functions drawing.h */
void gfxClear(Gfx);
int gfxRenderGlyph(Gfx, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
int gfxRenderElement(Gfx, uint16_t, uint16_t, const char*, uint16_t, uint16_t, uint16_t);
int gfxLayerChange(Gfx, const char*);
int gfxLayerPresent(Gfx, const char*);
int gfxRefresh(Gfx);

/* Input Functions input.h */
void gfxPollEvents(Gfx);
double gfxGetMouseX(void);
double gfxGetMouseY(void);
uint32_t gfxGetKey(void);
int getExitState(void);

#endif // VKTERM_PUBLIC_H
