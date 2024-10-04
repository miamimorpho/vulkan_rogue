#include "vulkan_helper.h"
#include "textures.h"
#include "drawing.h"
#include "input.h"
#include "config.h"

static GfxGlobal s_global;

// input.c
int inputInit(void){
  _inputInit(gfxGetConst());
  return 0;
}

// drawing.c
int gfxDrawStart(void){
  _gfxDrawStart(gfxGetConst(), &s_global);
  return 0;
}

int gfxDrawChar(uint32_t ch, int x, int y, int hex_color, int texture_index){
  _gfxDrawChar(gfxGetConst(), &s_global,
	       ch, x, y, hex_color, texture_index);
  return 0;
}

int gfxDrawString(const char* ch, int x, int y, int hex_color){
  _gfxDrawString(gfxGetConst(), &s_global,
		 ch, x, y, hex_color);
  return 0;
}

int gfxDrawEnd(void){
  _gfxDrawEnd(gfxGetConst(), &s_global);
  return 0;
}

// textures.c
int gfxTextureLoad(const char* filename){
  _gfxTextureLoad(gfxGetConst(), filename, s_global.textures);
  return 0;
}
GfxTileset gfxGetTexture(int texture_index){
  return s_global.textures[texture_index];
}

// local
int gfxGlobalInit(void){
  GfxConst gfx = gfxGetConst();
  _inputInit(gfx);
  _gfxTexturesInit(&s_global.textures);
  gfxVertBufferCreate(gfx, 30000,
		      &s_global.vertices);
  return 0;
}

int gfxConstInit(void){

  GfxConst* gfx = gfxSetConst();
  
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
  
  return 0;
}

int gfxClose(void){
  GfxConst gfx = gfxGetConst();

  // End last frame
  for(unsigned int i = 0; i < gfx.frame_c; i ++){
    vkWaitForFences(gfx.ldev, 1, &gfx.fence[i], VK_TRUE, UINT64_MAX);
    vkResetCommandBuffer(gfx.cmd_buffer[i], 0);
  }
  
  gfxBufferDestroy(gfx.allocator, &s_global.vertices);
  for(int i = 0; i < MAX_SAMPLERS; i++){
    if(s_global.textures[i].image.handle != VK_NULL_HANDLE){
      gfxImageDestroy(gfx.allocator, s_global.textures[i].image);
    }
  }
  _gfxConstFree(gfx);
  return 0;
}
