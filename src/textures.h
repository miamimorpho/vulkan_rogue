#ifndef TEXTURES_H
#define TEXTURES_H

#include "vulkan_meta.h"
#include <stdlib.h>

int _gfxTexturesInit(GfxTileset**);
int _gfxTextureLoad(GfxConst, const char*, GfxTileset*);

#endif //TEXTURES_H
