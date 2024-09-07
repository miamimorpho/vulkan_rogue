#include "vulkan_meta.h"

static GfxConst s_global;

GfxConst* gfxSetConst(void){
  return &s_global;
}

GfxConst gfxGetConst(void){
  return s_global;
}

VkCommandBuffer gfxCmdSingleBegin(void)
{
  VkCommandBufferAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandPool = s_global.cmd_pool,
    .commandBufferCount = 1,
  };

  VkCommandBuffer command_buffer;
  vkAllocateCommandBuffers(s_global.ldev, &alloc_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  vkBeginCommandBuffer(command_buffer, &begin_info);

  return command_buffer;
}

int gfxCmdSingleEnd(VkCommandBuffer cmd_buffer)
{
  vkEndCommandBuffer(cmd_buffer);

  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd_buffer,
  };

  if(vkQueueSubmit(s_global.queue, 1, &submit_info, VK_NULL_HANDLE)
     != VK_SUCCESS) return 1;
  
  vkQueueWaitIdle(s_global.queue);
  vkFreeCommandBuffers(s_global.ldev, s_global.cmd_pool, 1, &cmd_buffer);
  return 0;
}

int _gfxConstFree(GfxConst gfx){

  // destroy sync objects
  for(unsigned int i = 0; i < gfx.frame_c; i++){
    vkDestroySemaphore(gfx.ldev, gfx.present_bit[i], NULL);
    vkDestroySemaphore(gfx.ldev, gfx.render_bit[i], NULL);
    vkDestroyFence(gfx.ldev, gfx.fence[i], NULL);
  }

  free(gfx.present_bit);
  free(gfx.render_bit);
  free(gfx.fence);
  free(gfx.cmd_buffer);

  vkDestroyCommandPool(gfx.ldev, gfx.cmd_pool, NULL);
  vkDestroyPipelineLayout(gfx.ldev, gfx.pipeline_layout, NULL);
  vkDestroyPipeline(gfx.ldev, gfx.pipeline, NULL);
  vkDestroyDescriptorSetLayout(gfx.ldev, gfx.texture_descriptors_layout, NULL);
  vkDestroyDescriptorPool(gfx.ldev, gfx.descriptor_pool, NULL);
  
  for(uint32_t i = 0; i < gfx.swapchain_c; i++){
    vkDestroyFramebuffer(gfx.ldev, gfx.framebuffer[i], NULL);
  }
  free(gfx.framebuffer);
    
  vkDestroyRenderPass(gfx.ldev, gfx.renderpass, NULL);
  
  for(uint32_t i = 0; i < gfx.swapchain_c; i++)
    vkDestroyImageView(gfx.ldev, gfx.swapchain_views[i], NULL);

  free(gfx.swapchain_images);
  free(gfx.swapchain_views);

  vkDestroySwapchainKHR(gfx.ldev, gfx.swapchain, NULL);

  vmaDestroyAllocator(gfx.allocator);
  
  vkDestroyDevice(gfx.ldev, NULL);
  vkDestroySurfaceKHR(gfx.instance, gfx.surface, NULL);
  vkDestroyInstance(gfx.instance, NULL);

  glfwDestroyWindow(gfx.window);
  glfwTerminate();
  
  return 0;

}
