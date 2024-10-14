#include "vulkan_helper.h"
#include "textures.h"
#include "drawing.h"
#include "input.h"
#include "config.h"
#include <stdio.h>

static GfxGlobal s_global;

// input.c
int inputInit(void){
  _inputInit(gfxGetContext());
  return 0;
}

// drawing.c
int gfxDrawStart(void){
  _gfxDrawStart(gfxGetContext(), &s_global);
  return 0;
}

int gfxDrawChar(uint32_t ch, uint32_t x, uint32_t y,
		uint32_t fg, uint32_t bg, uint32_t texture_index){
  _gfxDrawChar(gfxGetContext(), &s_global,
	       ch, x, y, fg, bg, texture_index);
  return 0;
}

int gfxDrawString(const char* ch, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg){
  _gfxDrawString(gfxGetContext(), &s_global,
		 ch, x, y, fg, bg);
  return 0;
}

int gfxDrawEnd(void){
  _gfxDrawEnd(gfxGetContext(), &s_global);
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
int gfxGlobalInit(void){
  GfxContext gfx = gfxGetContext();
  _inputInit(gfx);
  _gfxTexturesInit(&s_global.textures);
  gfxVertBufferCreate(gfx, 30000,
		      &s_global.vertices);
  return 0;
}

int gfxConstInit(void){

  GfxContext* gfx = gfxSetContext();
  
  gfxGlfwInit(gfx);
  gfxInstanceInit(gfx);
  gfxPhysicalDeviceInit(gfx);
  gfxQueueIndex(gfx);
  gfxLogicalDeviceInit(gfx);
  gfxAllocatorInit(gfx);
  gfxCmdPoolInit(gfx);
  gfxSurfaceInit(gfx);
  gfxSwapchainInit(gfx);
  gfxRenderpassInit(gfx);
  gfxFramebufferInit(gfx);
  gfxDescriptorsPool(gfx);
  gfxSyncInit(gfx);
  gfxCmdBuffersInit(gfx);
  gfxTextureDescriptorsInit(gfx);
  gfxPipelineInit(gfx );

  VkPhysicalDeviceProperties devProperties;
  vkGetPhysicalDeviceProperties(gfx->pdev, &devProperties);
  printf("Physical Device Name: %s %p\n", devProperties.deviceName, (void*)gfx->pdev);
  
  return 0;
}

int gfxClose(void){
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
