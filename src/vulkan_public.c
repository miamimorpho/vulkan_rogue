#include "vulkan_meta.h"
#include "textures.h"
#include "drawing.h"
#include "input.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

// local
int gfxScreenInit(GfxGlobal* global){

  GfxContext* vulkan = &global->vk;
  gfxGlfwInit(vulkan);
  gfxInputInit(vulkan->window);
  gfxInstanceInit(vulkan);
  gfxPhysicalDeviceInit(vulkan);
  gfxQueueIndex(vulkan);
  gfxLogicalDeviceInit(vulkan);
  init_vkCmdBeginRenderingKHR(vulkan->ldev);

  gfxAllocatorInit(vulkan);
  gfxCmdPoolInit(vulkan);
  gfxSurfaceInit(vulkan);
  gfxSwapchainInit(vulkan);
  
  gfxDescriptorsPool(vulkan);
  gfxSyncInit(vulkan);
  gfxCmdBuffersInit(vulkan);
  gfxTextureDescriptorsInit(vulkan);
  gfxPipelineInit(vulkan);
  gfxTexturesInit(&global->textures);

  global->swapchain_x = 0;
  global->frame_x = 0;
  
  // Create Scratch Buffers
  int tile_buffer_size = ASCII_SCREEN_WIDTH * ASCII_SCREEN_HEIGHT;
  gfxBufferCreate(vulkan->allocator,
		  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		  5 * tile_buffer_size * sizeof(GfxGlyph), &global->gpu_glyph_cache);  
  global->caches = malloc(sizeof(GfxCache));
  global->cache_c = 0;
  gfxCacheChange(global, "main");
  // Create Draw Buffers
  gfxBufferCreate(vulkan->allocator,
		  VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		  5 * sizeof(VkDrawIndirectCommand), &global->indirect);
  
  return 0;
}

int gfxScreenClose(GfxGlobal global){

  GfxContext vk = global.vk;
  
  // End last frame
  VkResult result;
  for(unsigned int i = 0; i < vk.frame_c; i ++){
    result = vkWaitForFences(vk.ldev, 1, &vk.fence[i], VK_TRUE, UINT32_MAX);
    if(result == VK_TIMEOUT){
      printf("FATAL: VkWaitForFences timed out\n");
      // protection from deadlocks
      abort();
    }
    vkResetCommandBuffer(vk.cmd_buffer[i], 0);
  }

  // free global back bugger
  gfxBufferDestroy(vk.allocator, &global.gpu_glyph_cache);
  gfxBufferDestroy(vk.allocator, &global.indirect);
  
  // free asset data
  for(int i = 0; i < MAX_SAMPLERS; i++){
    if(global.textures[i].image.handle != VK_NULL_HANDLE){
      gfxImageDestroy(vk.allocator, global.textures[i].image);
    }
  }

  // free vulkan
  _gfxConstFree(vk);
  return 0;
}
