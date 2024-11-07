#ifndef VTERM_PUBLIC_H
#define VTERM_PUBLIC_H
#include "config.h"
#include "gfx_input.h"
#include <stdint.h>

struct GfxGlobal;
typedef struct GfxGlobal* Gfx;

/* Init + Settings */
Gfx gfxScreenInit(void);
int gfxTextureLoad(Gfx, const char*);
int gfxScreenClose(Gfx);

/* Drawing Functions */
void gfxClear(Gfx);
int gfxAddCh(Gfx, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
int gfxAddString(Gfx, uint16_t, uint16_t, const char*, uint16_t, uint16_t);
int gfxCacheChange(Gfx, const char*);
int gfxCachePresent(Gfx, const char*);
int gfxRefresh(Gfx);

/* Input Functions */
void gfxMousePos(int*, int*);
void gfxPollEvents(Gfx);
uint32_t gfxInputUnicode(void);
int getExitState(void);

#endif // VTERM_PUBLIC_H
