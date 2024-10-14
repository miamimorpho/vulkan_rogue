#include "vulkan_helper.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define TOP_LEFT_INDEX 0
#define TOP_RIGHT_INDEX 1
#define BOTTOM_LEFT_INDEX 2
#define BOTTOM_RIGHT_INDEX 3

int _gfxDrawStart(GfxContext gfx, GfxGlobal* global){
  
  vkWaitForFences(gfx.ldev, 1,
		  &gfx.fence[global->frame_x],
		  VK_TRUE, UINT32_MAX);

  VkSemaphore* present_bit = &gfx.present_bit[global->frame_x];
  VkResult result =
    vkAcquireNextImageKHR(gfx.ldev, gfx.swapchain, UINT64_MAX,
			  *present_bit,
			  VK_NULL_HANDLE,
			  &global->swapchain_x);
 
  if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    gfxRecreateSwapchain();
    vkDestroySemaphore(gfx.ldev, *present_bit, NULL);
    VkSemaphoreCreateInfo semaphore_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VK_CHECK(vkCreateSemaphore(gfx.ldev, &semaphore_info, NULL, present_bit));
    return _gfxDrawStart(gfxGetContext(), global);
  }

  vkResetFences(gfx.ldev, 1, &gfx.fence[global->frame_x]);
  
  VkCommandBuffer cmd_b = gfx.cmd_buffer[global->frame_x];
  vkResetCommandBuffer(cmd_b, 0);

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = 0,
    .pInheritanceInfo = NULL,
  };

  if(vkBeginCommandBuffer(cmd_b, &begin_info) != VK_SUCCESS){
    printf("!failed to begin recording command buffer!\n");
  }

  VkClearValue clears[2];
  clears[0].color = (VkClearColorValue){ { 0.0f, 0.0f, 0.0f, 1.0f } };
  clears[1].depthStencil = (VkClearDepthStencilValue){ 1.0f, 0 };
 
  VkRenderPassBeginInfo render_pass_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = gfx.renderpass,
    .framebuffer = gfx.framebuffer[global->swapchain_x],
    .renderArea.offset = {0, 0},
    .renderArea.extent = gfx.extent,
    .clearValueCount = 2,
    .pClearValues = clears,
  };
    
  vkCmdBeginRenderPass(cmd_b, &render_pass_info,
		       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindDescriptorSets(cmd_b, VK_PIPELINE_BIND_POINT_GRAPHICS,
			  gfx.pipeline_layout, 0, 1,
			  &gfx.texture_descriptors, 0, NULL);
  
  return 0;
}

int _gfxAddQuad(GfxContext gfx, GfxGlobal* global, vertex2 vertices[4]){
 
  int buffer_offset = global->vertices.used_size / sizeof(vertex2);
  gfxBufferAppend(gfx.allocator, &global->vertices, vertices, 4 * sizeof(vertex2));
  
  uint32_t indices[6];
  indices[0] = TOP_LEFT_INDEX + buffer_offset;
  indices[1] = BOTTOM_LEFT_INDEX + buffer_offset;
  indices[2] = TOP_RIGHT_INDEX + buffer_offset;
  
  indices[3] = TOP_RIGHT_INDEX + buffer_offset;
  indices[4] = BOTTOM_LEFT_INDEX + buffer_offset;
  indices[5] = BOTTOM_RIGHT_INDEX + buffer_offset;

  GfxBuffer* dest_indices = gfxBufferNext
    (gfx.allocator, &global->vertices);
  gfxBufferAppend(gfx.allocator, dest_indices, indices, 6 * sizeof(uint32_t));

  return 0;
}

int _gfxDrawChar(GfxContext gfx, GfxGlobal* global, uint32_t ch, uint32_t x, uint32_t y,
		 uint32_t fg, uint32_t bg, uint32_t texture_i){
  
  GfxTileset texture = global->textures[texture_i];
  if(texture.image.handle == NULL){
    return 1;
  }
  
  // ncurses space to screen space
  vec2 stride;
  stride.x = (2 * ASCII_SCALE * ASCII_TILE_SIZE) / (float)gfx.extent.width;
  stride.y = (2 * ASCII_SCALE * ASCII_TILE_SIZE) / (float)gfx.extent.height;

  vec2 uv_stride;
  uv_stride.x = (float)texture.glyph_width /
    (float)texture.width;
  uv_stride.y = (float)texture.glyph_height /
    (float)texture.height;

  int width_in_tiles = texture.width / texture.glyph_width;
  vec2 uv_index;
  uv_index.x = (float)(ch % width_in_tiles) * uv_stride.x;
  uv_index.x += (float)texture_i;
  uv_index.y = (float)(ch / width_in_tiles) * uv_stride.y;


  vec2 cursor;
  cursor.x = -1 + (stride.x * (float)x);
  cursor.y = -1 + (stride.y * (float)y); 
  
  vertex2 vertices[4];
  vertices[TOP_LEFT_INDEX] = (vertex2){
    .pos = cursor,
    .uv = uv_index,
    .fgIndex_bgIndex = (uint32_t)fg << 16 | (uint32_t)bg
  };

  vertices[BOTTOM_LEFT_INDEX] = vertices[TOP_LEFT_INDEX];
  vertices[BOTTOM_LEFT_INDEX].pos.y += stride.y;
  vertices[BOTTOM_LEFT_INDEX].uv.y += uv_stride.y;
    
  vertices[TOP_RIGHT_INDEX] = vertices[TOP_LEFT_INDEX];
  vertices[TOP_RIGHT_INDEX].pos.x += stride.x;
  vertices[TOP_RIGHT_INDEX].uv.x += uv_stride.x;
  
  vertices[BOTTOM_RIGHT_INDEX] = vertices[TOP_RIGHT_INDEX];
  vertices[BOTTOM_RIGHT_INDEX].pos.y += stride.y;
  vertices[BOTTOM_RIGHT_INDEX].uv.y += uv_stride.y;

  _gfxAddQuad(gfx, global, vertices);
  return 0;
}

int _gfxDrawString(GfxContext gfx, GfxGlobal* global, const char* str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg){

  //convert to unicode
  
  int i = 0;
  int start_x = 0;
  while(str[i] != '\0') {
    if(str[i] == '\n'){
      y++;
      x = start_x;
    }else{
      int uv = getUnicodeUV(global->textures[ASCII_TEXTURE_INDEX], str[i]);
      int err = _gfxDrawChar(gfx, global, uv,
			     x++, y, fg, bg, ASCII_TEXTURE_INDEX);
      if(err != 0){
	return 1;
      }
    }
    i++;
  }
  return 0;
}

int _gfxDrawEnd(GfxContext gfx, GfxGlobal* global){

  VkCommandBuffer cmd_b = gfx.cmd_buffer[global->frame_x];
  
  VkViewport viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = gfx.extent.width,
    .height = gfx.extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  vkCmdSetViewport(cmd_b, 0, 1, &viewport);

  VkRect2D scissor = {
    .offset = {0, 0},
    .extent = gfx.extent,
  };
    
  vkCmdSetScissor(cmd_b, 0, 1, &scissor);
  vkCmdBindPipeline(cmd_b, VK_PIPELINE_BIND_POINT_GRAPHICS,
	            gfx.pipeline);

  
  VmaAllocationInfo g_vertices_info;
  vmaGetAllocationInfo(gfx.allocator, global->vertices.allocation, &g_vertices_info);
  GfxBuffer* g_indices = (GfxBuffer*)g_vertices_info.pUserData;

  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(cmd_b,
			 0, 1, &global->vertices.handle, offsets);
  vkCmdBindIndexBuffer(cmd_b,
		       g_indices->handle,
		       0, VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(cmd_b, g_indices->used_size / sizeof(uint32_t),
		   1, 0, 0, 0);
  global->vertices.used_size = 0;
  g_indices->used_size = 0;

  // draw end
  vkCmdEndRenderPass(cmd_b);
  if(vkEndCommandBuffer(cmd_b) != VK_SUCCESS) {
    printf("!failed to record command buffer!");
    return 1;
  }
  
  VkSemaphore wait_semaphores[] = { gfx.present_bit[global->frame_x] };
  VkPipelineStageFlags wait_stages[] =
    {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSemaphore signal_semaphores[] = { gfx.render_bit[global->frame_x] };

  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = wait_semaphores,
    .pWaitDstStageMask = wait_stages,
    .commandBufferCount = 1,
    .pCommandBuffers = &gfx.cmd_buffer[global->frame_x],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = signal_semaphores,
  };

  if(vkQueueSubmit(gfx.queue, 1, &submit_info, gfx.fence[global->frame_x])
     != VK_SUCCESS) {
    printf("!failed to submit draw command buffer!\n");
    return 1;
  }
    
  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = signal_semaphores,
    .swapchainCount = 1,
    .pSwapchains = &gfx.swapchain,
    .pImageIndices = &global->swapchain_x,
    .pResults = NULL
  };
   
  VkResult results = vkQueuePresentKHR(gfx.queue, &present_info);
  if(results != VK_SUCCESS) {
    return 1;
  }
  
  return 0;
}
