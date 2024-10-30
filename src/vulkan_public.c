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
int gfxRenderFrame(void){
  return _gfxRenderFrame(gfxGetContext(), &s_global);
}

int gfxCacheChange(const char* name){
  return _gfxCacheChange(&s_global, name);
}

int gfxDrawCache(const char* name){
  return _gfxDrawCache(gfxGetContext(), &s_global, name);
}

int gfxAddCh(uint16_t x, uint16_t y, uint16_t encoding, uint16_t texture_index, uint16_t fg, uint16_t bg){

  return _gfxAddCh(&s_global, x, y,
		   encoding, texture_index,
		   fg, bg);
}

int gfxAddString(uint32_t x, uint32_t y, const char* str, uint32_t fg, uint32_t bg){
  _gfxAddString(gfxGetContext(), &s_global,
		 x, y, str, fg, bg);
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
  gfxDescriptorsPool(p_gfx);
  gfxSyncInit(p_gfx);
  gfxCmdBuffersInit(p_gfx);
  gfxTextureDescriptorsInit(p_gfx);
  gfxPipelineInit(p_gfx);

  GfxContext gfx = gfxGetContext();

  init_vkCmdBeginRenderingKHR(gfx.ldev);
  
  _inputInit(gfx);
  _gfxTexturesInit(&s_global.textures);

  // Create Scratch Buffers
  s_global.caches = malloc(sizeof(TileDrawCache));
  cacheCreate(&s_global.caches[0], "main", LONG);
  s_global.cache_x = 0;
  s_global.cache_c = 1;

  // Create Draw Buffers
  int tile_buffer_size = ASCII_SCREEN_WIDTH * ASCII_SCREEN_HEIGHT;
  
  gfxBufferCreate(gfx.allocator,
		  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		  tile_buffer_size * sizeof(TileDrawInstance), &s_global.tile_draw_instances);  
  return 0;
}

int gfxScreenClose(void){
  GfxContext gfx = gfxGetContext();

  // End last frame
  VkResult result;
  for(unsigned int i = 0; i < gfx.frame_c; i ++){
    result = vkWaitForFences(gfx.ldev, 1, &gfx.fence[i], VK_TRUE, UINT32_MAX);
    if(result == VK_TIMEOUT){
      printf("FATAL: VkWaitForFences timed out\n");
      // protection from deadlocks
      abort();
    }
    vkResetCommandBuffer(gfx.cmd_buffer[i], 0);
  }

  // free global back bugger
  gfxBufferDestroy(gfx.allocator, &s_global.tile_draw_instances);
  
  // free asset data
  for(int i = 0; i < MAX_SAMPLERS; i++){
    if(s_global.textures[i].image.handle != VK_NULL_HANDLE){
      gfxImageDestroy(gfx.allocator, s_global.textures[i].image);
    }
  }

  // free vulkan
  _gfxConstFree(gfx);
  return 0;
}
