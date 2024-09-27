#include "vulkan_helper.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

ivec3 hexColor(uint32_t color) {
    ivec3 rgb;
    rgb.x = ((color >> 16) & 0xFF);
    rgb.y = ((color >> 8) & 0xFF);
    rgb.z = (color & 0xFF);
    return rgb;
}

int _gfxDrawStart(GfxConst gfx, GfxGlobal* global){
  
  VkFence fences[] = { gfx.fence[global->frame_x] };
  vkWaitForFences(gfx.ldev, 1, fences, VK_TRUE, UINT64_MAX);
  vkResetFences(gfx.ldev, 1, fences);

  vkAcquireNextImageKHR(gfx.ldev, gfx.swapchain, UINT64_MAX,
			gfx.present_bit[global->frame_x],
			VK_NULL_HANDLE,
			&global->swapchain_x);
 
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

int gfxBufferAppend(VmaAllocator allocator, GfxBuffer *dest,
		    const void* src, VkDeviceSize src_size)
{
  VmaAllocationInfo dest_info;
  vmaGetAllocationInfo(allocator, dest->allocation, &dest_info);
  
  if(dest->used_size + src_size > dest_info.size)
    return 1;
  
  vmaCopyMemoryToAllocation(allocator, src,
			    dest->allocation, dest->used_size,
			    src_size);
  dest->used_size += src_size;
  return 0;
}

int _gfxDrawTexture(GfxConst gfx, GfxGlobal* global, int texture_index){

  const size_t vertex_c = 4;
  vertex2 vertices[vertex_c];
  const size_t index_c = 6;
  uint32_t indices[index_c];
  
  vertex2 top_left = {
    .pos = { 0, 0},
    .uv = { 0, 0},
    .color = hexColor(0xFFFFFF),
  };
  vertex2 bottom_left = top_left;
  bottom_left.pos.y += 1;
  bottom_left.uv.y += 1;
    
  vertex2 top_right = top_left;
  top_right.pos.x += 1;
  top_right.uv.x += 1;
  vertex2 bottom_right = top_right;
  bottom_right.pos.y += 1;
  bottom_right.uv.y += 1;
  
  const int TL = 0;
  const int TR = 1;
  const int BL = 2;
  const int BR = 3;
  
  vertices[TL] = top_left;
  vertices[TR] = top_right;
  vertices[BL] = bottom_left;
  vertices[BR] = bottom_right;

  int buffer_offset = global->vertices.used_size / sizeof(vertex2);
 
  gfxBufferAppend(gfx.allocator, &global->vertices, vertices, vertex_c * sizeof(vertex2));
  GfxBuffer* dest_indices = gfxBufferNext
    (gfx.allocator, &global->vertices);
  
  // get rid of indexed drawing at some point  
  indices[0] = TL + buffer_offset;
  indices[1] = BL + buffer_offset;
  indices[2] = TR + buffer_offset;
  
  indices[3] = TR + buffer_offset;
  indices[4] = BL + buffer_offset;
  indices[5] = BR + buffer_offset;
  
  gfxBufferAppend(gfx.allocator, dest_indices, indices, index_c * sizeof(uint32_t));
  
  return 0;
}

int _gfxDrawChar(GfxConst gfx, GfxGlobal* global, uint16_t ch, int x, int y, int hex_color){

  ivec3 color = hexColor(hex_color);
  
  GfxTileset texture = global->textures[0];
  // ncurses space to screen space
  vec2 stride;
  stride.x = (6.0f * texture.glyph_width) / (float)gfx.extent.width;
  stride.y = (6.0f * texture.glyph_height) / (float)gfx.extent.height;

  int width_in_tiles = texture.width / texture.glyph_width;
  vec2 uv_stride;
  uv_stride.x = (float)texture.glyph_width /
    (float)texture.width;
  uv_stride.y = (float)texture.glyph_height /
    (float)texture.height;
  
  const size_t vertex_c = 4;
  vertex2 vertices[vertex_c];
  const size_t index_c = 6;
  uint32_t indices[index_c];
  
  float cursor_x = -1 + (stride.x * (float)x);
  float cursor_y = -1 + (stride.y * (float)y); 
  
  vertex2 top_left = {
    .pos = { cursor_x, cursor_y},
    .uv = { (float)(ch % width_in_tiles) * uv_stride.x,
	    (float)(ch / width_in_tiles) * uv_stride.y},
    .color = color
  };

  vertex2 bottom_left = top_left;
  bottom_left.pos.y += stride.y;
  bottom_left.uv.y += uv_stride.y;
    
  vertex2 top_right = top_left;
  top_right.pos.x += stride.x;
  top_right.uv.x += uv_stride.x;
  
  vertex2 bottom_right = top_right;
  bottom_right.pos.y += stride.y;
  bottom_right.uv.y += uv_stride.y;
  
  const int TL = 0;
  const int TR = 1;
  const int BL = 2;
  const int BR = 3;
  
  vertices[TL] = top_left;
  vertices[TR] = top_right;
  vertices[BL] = bottom_left;
  vertices[BR] = bottom_right;

  int buffer_offset = global->vertices.used_size / sizeof(vertex2);
 
  gfxBufferAppend(gfx.allocator, &global->vertices, vertices, vertex_c * sizeof(vertex2));
  GfxBuffer* dest_indices = gfxBufferNext
    (gfx.allocator, &global->vertices);
  
  // get rid of indexed drawing at some point  
  indices[0] = TL + buffer_offset;
  indices[1] = BL + buffer_offset;
  indices[2] = TR + buffer_offset;
  
  indices[3] = TR + buffer_offset;
  indices[4] = BL + buffer_offset;
  indices[5] = BR + buffer_offset;
  
  gfxBufferAppend(gfx.allocator, dest_indices, indices, index_c * sizeof(uint32_t));

  return 0;
}

int _gfxDrawEnd(GfxConst gfx, GfxGlobal* global){

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

  VkBuffer vertex_buffers[] = {global->vertices.handle};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(cmd_b,
			 0, 1, vertex_buffers, offsets);
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
