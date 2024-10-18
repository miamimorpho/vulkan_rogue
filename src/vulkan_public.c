#include "vulkan_meta.h"
#include "textures.h"
#include "drawing.h"
#include "input.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

static GfxGlobal s_global;

// input.c
int inputInit(void){
  _inputInit(gfxGetContext());
  return 0;
}

// drawing.c
int gfxRefresh(void){
  _gfxRefresh(gfxGetContext(), &s_global);
  return 0;
}

int gfxAddCh(uint16_t x, uint16_t y, uint16_t glyph_code, uint16_t fg_index, uint16_t bg_index, uint16_t texture_index){

  return _gfxAddCh(&s_global,
		   x, y, glyph_code, fg_index, bg_index,
		   texture_index);
}

int gfxDrawString(const char* ch, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg){
  _gfxDrawString(gfxGetContext(), &s_global,
		 ch, x, y, fg, bg);
  return 0;
}

// textures.c
int gfxTextureLoad(const char* filename){
  _gfxTextureLoad(gfxGetContext(), filename, s_global.textures);
  return 0;
}
GfxTileset gfxGetTexture(int texture_index){
  return s_global.textures[texture_index];
}

// local
int gfxScreenInit(void){
  GfxContext* p_gfx = gfxSetContext();

  gfxGlfwInit(p_gfx);
  gfxInstanceInit(p_gfx);
  gfxPhysicalDeviceInit(p_gfx);
  gfxQueueIndex(p_gfx);
  gfxLogicalDeviceInit(p_gfx);
  gfxAllocatorInit(p_gfx);
  gfxCmdPoolInit(p_gfx);
  gfxSurfaceInit(p_gfx);
  gfxSwapchainInit(p_gfx);
  gfxRenderpassInit(p_gfx);
  gfxFramebufferInit(p_gfx);
  gfxDescriptorsPool(p_gfx);
  gfxSyncInit(p_gfx);
  gfxCmdBuffersInit(p_gfx);
  gfxTextureDescriptorsInit(p_gfx);
  gfxPipelineInit(p_gfx);

  GfxContext gfx = gfxGetContext();
  
  _inputInit(gfx);
  _gfxTexturesInit(&s_global.textures);
  gfxVertBufferCreate(gfx, 10000,
		      &s_global.vertices);
  
  s_global.tile_buffer_w = ASCII_SCREEN_WIDTH;
  s_global.tile_buffer_h = ASCII_SCREEN_HEIGHT;
  int tile_buffer_size = ASCII_SCREEN_WIDTH * ASCII_SCREEN_HEIGHT;
  s_global.tile_buffer =
    malloc(tile_buffer_size * sizeof(GfxTile));

  GfxTile null_tile = {
    .glyph_code = 0,
    .fg_index = 0,
    .bg_index = 0,
    .texture_index = 0,
  };
  for(int i = 0; i < tile_buffer_size; i++){
    s_global.tile_buffer[i] = null_tile;
  }
  return 0;
}

int gfxScreenClose(void){
  GfxContext gfx = gfxGetContext();

  // End last frame
  for(unsigned int i = 0; i < gfx.frame_c; i ++){
    vkWaitForFences(gfx.ldev, 1, &gfx.fence[i], VK_TRUE, UINT64_MAX);
    vkResetCommandBuffer(gfx.cmd_buffer[i], 0);
  }

  // free asset data
  gfxBufferDestroy(gfx.allocator, &s_global.vertices);
  for(int i = 0; i < MAX_SAMPLERS; i++){
    if(s_global.textures[i].image.handle != VK_NULL_HANDLE){
      gfxImageDestroy(gfx.allocator, s_global.textures[i].image);
    }
  }

  // free vulkan
  _gfxConstFree(gfx);
  return 0;
}
