#include "macros.h"
#include "vkterm_private.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct{
  float x;
  float y;
} vec2;

typedef struct{
  vec2 screen_size_px;
}GfxPushConstant;

uint32_t pack16into32(uint16_t a, uint16_t b){
  return(uint32_t)a << 16 | (uint32_t)b;
}

int layerSearch(GfxLayer* layers, int layer_c, const char* name){
  if(layer_c == 0) return -1;
  int found_buffer = -1;
  for(int i = 0; i < layer_c; i++){
    if(layers[i].type == LAYER_TYPE_FREE) break;
    if(strcmp(name, layers[i].name) == 0){
      found_buffer = i;
      break;
    }
  }
  return found_buffer;
}

int layerCountGrow(GfxLayer** layer_arr_ptr, int* layer_c){
  int new_layer_c = 0;
  if(*layer_c == 0){
    new_layer_c = 1;
  }else{
    new_layer_c = *layer_c * 2;
  }
  GfxLayer* new_layer_arr = realloc(*layer_arr_ptr, new_layer_c * sizeof(GfxLayer));
  if(new_layer_arr == NULL) return -1;
  
  for(int i = *layer_c; i < new_layer_c; i++){
    GfxLayer* layer_i = &new_layer_arr[i];
    layer_i->type = LAYER_TYPE_FREE;
    layer_i->name = NULL;
  }
  *layer_c = new_layer_c;
  *layer_arr_ptr = new_layer_arr;
  return 0;
}

void layerBufferDestroy(GfxGlobal* gfx){
  for(int i = 0; i < gfx->layer_c; i++){
      free(gfx->layers[i].name);
      free(gfx->layers[i].data);
  }
  free(gfx->layers);
}

void gfxClear( GfxGlobal* gfx ){
  GfxLayer* layer = &gfx->layers[gfx->layer_x];
  if(layer != NULL){
      memset(layer->data, 0, layer->count * sizeof(GfxGlyph));
  }
}

int layerCreate(GfxLayer* dst, const char* name){

  size_t name_len = strlen(name) +1;
  dst->name = malloc(name_len);
  if(dst->name != NULL){
    memcpy(dst->name, name, name_len);
  }

  dst->type = LAYER_TYPE_USED;
  dst->count = ASCII_SCREEN_WIDTH * ASCII_SCREEN_HEIGHT;
  dst->data = (GfxGlyph*)malloc(dst->count * sizeof(GfxGlyph));
  memset(dst->data, 0, dst->count * sizeof(GfxGlyph));

  return 0;
}

int gfxLayerSendToGPU(GfxGlobal gfx, int i){
  if(i > gfx.layer_c) return 1;
  GfxLayer* layer = &gfx.layers[i];
  
  const int glyph_c = ASCII_SCREEN_WIDTH * ASCII_SCREEN_HEIGHT;
  const int src_size = glyph_c * sizeof(GfxGlyph);

  if(layer->data == NULL){
    printf("fatal\n");
    return 1;
  }
    
  int res = vmaCopyMemoryToAllocation
      (gfx.vk.allocator, layer->data,
       gfx.gpu_glyph_cache.allocation,
       src_size * i,
       src_size );
  VK_CHECK(res);
			    
  return 0;
}

int gfxLayerChange(GfxGlobal* gfx, const char* name){

  /* get cache index by searching string
   * if it does not exist, alloc and create index
   */  
  int layer_x = layerSearch(gfx->layers, gfx->layer_c, name);
  if(layer_x == -1){
    // cache not found, find empty slot
    int free_layer_x = -1;
    if(gfx->layer_c > 0){
      for(int i = 0; i < gfx->layer_c; i++){
          if(gfx->layers[i].type == LAYER_TYPE_FREE){
              free_layer_x = i;
          }
      }
    }

    if(free_layer_x == -1){
      // no empty slots, realloc, try again
      layerCountGrow(&gfx->layers, &gfx->layer_c);
      return gfxLayerChange(gfx, name);
    }
    // init new cache
    GfxLayer* new_layer_ptr = &gfx->layers[free_layer_x];
    layerCreate(new_layer_ptr, name);
    return gfxLayerChange(gfx, name);
  }else{
    /* upload whatever has been drawn to this point
     * onto the GPU */
    gfxLayerSendToGPU(*gfx, gfx->layer_x);
    gfx->layer_x = layer_x;
  }
  return 0;
}

uint32_t pack_indices(uint16_t unicode, uint8_t atlas_index,
		      uint8_t fg_index, uint8_t bg_index) {
  return (uint32_t)((unicode & 0x3FF) << 22) | 
    ((atlas_index & 0x3F) << 16) | 
    (fg_index << 8) | 
    bg_index;
}

int gfxRenderGlyph(GfxGlobal* gfx, int32_t x, int32_t y,
	      uint16_t encoding, uint16_t atlas_index,
	      uint16_t fg, uint16_t bg){

  if(x >= ASCII_SCREEN_WIDTH || x < 0) return 1;
  if(y >= ASCII_SCREEN_HEIGHT || y < 0) return 1;

// TODO, make memory safe
  if(atlas_index > MAX_SAMPLERS) return 1;
  if(gfx->tilesets[atlas_index].image.handle == NULL){
    //fprintf(stderr, "gfxRenderGlyph texture_index err\n");
    atlas_index = ASCII_TEXTURE_INDEX;
  }
  GfxTileset tex = gfx->tilesets[atlas_index]; 
  uint16_t unicode_uv = tex.decoder(tex.encodings, tex.glyph_c, encoding);
  
  GfxGlyph dst = {
    .pos = pack16into32(x, y),
    .unicode_atlas_and_colors = pack_indices(unicode_uv, atlas_index, fg, bg)
  };
  GfxLayer* dst_layer = &gfx->layers[gfx->layer_x];
  dst_layer->data[y * ASCII_SCREEN_WIDTH + x] = dst;
  return 0;
}

int gfxRenderElement(GfxGlobal* gfx,
		 uint16_t x, uint16_t y,
         const char* str,
         uint16_t atlas_index,
		 uint16_t fg, uint16_t bg){
  int i = 0;
  int start_x = 0;
  
  while(str[i] != '\0') {
    if(str[i] == '\n'){
      y++;
      x = start_x;
    }else{
      gfxRenderGlyph(gfx, x++, y,
		str[i], atlas_index,
		fg, bg);
    }
    i++;
  }
  return 0;
}

int gfxLayerPresent(GfxGlobal* gfx, const char* name){
  int i = layerSearch(gfx->layers, gfx->layer_c, name);
  GfxLayer* layer = &gfx->layers[i];
  VkDrawIndirectCommand src = {
    .vertexCount = 6,
    .instanceCount = layer->count,
    .firstVertex = 0,
    .firstInstance = layer->count * i, // assumes every cache is same size
  };

  gfxBufferAppend(gfx->vk.allocator, &gfx->indirect, &src,
		  sizeof(VkDrawIndirectCommand));
  return 0;
}

int gfxBakeCommandBuffer(GfxGlobal* gfx)
{
  VkCommandBuffer cmd_b = gfx->vk.cmd_buffer[gfx->swapchain_x];
  vkResetCommandBuffer(cmd_b, 0);

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = 0,
    .pInheritanceInfo = NULL,
  };
  
  if(vkBeginCommandBuffer(cmd_b, &begin_info) != VK_SUCCESS){
    printf("!failed to begin recording command buffer!\n");
  }

  transitionImageLayout(cmd_b, gfx->vk.swapchain_images[gfx->swapchain_x],
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


  transitionImageLayout(cmd_b, gfx->vk.swapchain_images[gfx->swapchain_x],
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  
  VkClearValue clear_value;
  clear_value.color = 
    (VkClearColorValue){ { 0.0f, 0.0f, 0.0f, 1.0f } };
  
  VkRenderingAttachmentInfoKHR color_attachment_info = {
    VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
    .imageView = gfx->vk.swapchain_views[gfx->swapchain_x],
    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = clear_value,
  };
    
  VkRenderingInfoKHR render_info = {
    VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
    .renderArea.offset = {0, 0},
    .renderArea.extent = gfx->vk.extent,
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment_info,
  };
  pfn_vkCmdBeginRenderingKHR(cmd_b, &render_info);
			 
  GfxPushConstant constants = {
    .screen_size_px = (vec2){ gfx->vk.extent.width, gfx->vk.extent.height },
  };
  vkCmdPushConstants(cmd_b, gfx->vk.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GfxPushConstant), &constants);
  
  vkCmdBindDescriptorSets(cmd_b, VK_PIPELINE_BIND_POINT_GRAPHICS,
			  gfx->vk.pipeline_layout, 0, 1,
			  &gfx->vk.texture_descriptors, 0, NULL);
  
  VkViewport viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = gfx->vk.extent.width * ASCII_SCALE,
    .height = gfx->vk.extent.height * ASCII_SCALE,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  vkCmdSetViewport(cmd_b, 0, 1, &viewport);
  
  VkRect2D scissor = {
    .offset = {0, 0},
    .extent = gfx->vk.extent,
  };
  
  vkCmdSetScissor(cmd_b, 0, 1, &scissor);
  vkCmdBindPipeline(cmd_b, VK_PIPELINE_BIND_POINT_GRAPHICS,
		    gfx->vk.pipeline);
  
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(cmd_b, 0, 1,
			 &gfx->gpu_glyph_cache.handle, offsets);

  vkCmdDrawIndirect(cmd_b, gfx->indirect.handle, 0,
  gfx->indirect.used_size / sizeof(VkDrawIndirectCommand),
  sizeof(VkDrawIndirectCommand));
   
  // draw end
  pfn_vkCmdEndRenderingKHR(cmd_b);
  
  if(vkEndCommandBuffer(cmd_b) != VK_SUCCESS) {
    printf("!failed to record command buffer!");
    return 1;
  }
  return 0;
}

int gfxRefresh(GfxGlobal* gfx){

  GfxContext vk = gfx->vk;
  
  // waits on last VkQueueSubmit to be done
  VkResult fence_result;
  fence_result = vkWaitForFences(vk.ldev, 1,
		    &vk.fence[gfx->frame_x],
		    VK_TRUE, UINT32_MAX);
  if(fence_result == VK_TIMEOUT){
    printf("FATAL: VkWaitForFences timed out\n");
    abort();
  }
  vkResetFences(vk.ldev, 1, &vk.fence[gfx->frame_x]);
  
  VkSemaphore* image_available = &vk.image_available[gfx->frame_x];  
  VkResult result = VK_TIMEOUT;
  result = vkAcquireNextImageKHR(vk.ldev, vk.swapchain, UINT32_MAX,
				 *image_available,
				 VK_NULL_HANDLE,
				 &gfx->swapchain_x);
  if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    gfxRecreateSwapchain(&gfx->vk);
    vkDestroySemaphore(vk.ldev, *image_available, NULL);
    VkSemaphoreCreateInfo semaphore_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VK_CHECK(vkCreateSemaphore(vk.ldev, &semaphore_info, NULL, image_available));
    return 1;
  }

  // upload last drawn cache to GPU
  gfxLayerSendToGPU(*gfx, gfx->layer_x);
  gfxBakeCommandBuffer(gfx);
  
  VkPipelineStageFlags wait_stages[] =
    {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
 
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = image_available,
    .pWaitDstStageMask = wait_stages,
    .commandBufferCount = 1,
    .pCommandBuffers = &vk.cmd_buffer[gfx->swapchain_x],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &vk.render_finished[gfx->frame_x],
  };

  // waits on VkAcquireImageKHR to signal an image is available
  VK_CHECK(vkQueueSubmit(vk.queue, 1, &submit_info, vk.fence[gfx->frame_x]));

  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &vk.render_finished[gfx->frame_x],
    .swapchainCount = 1,
    .pSwapchains = &vk.swapchain,
    .pImageIndices = &gfx->swapchain_x,
    .pResults = NULL
  };

  // waits on vkQueueSubmit to signal its finished rendering
  VkResult present_result = vkQueuePresentKHR(vk.queue, &present_info);
  switch(present_result){
  case VK_SUCCESS:
    break;
  case VK_ERROR_OUT_OF_DATE_KHR:
    break;
  case VK_SUBOPTIMAL_KHR:
    break;
  default:
    return 1;
  }

  gfx->frame_x = (gfx->frame_x + 1) % vk.frame_c;
  gfx->indirect.used_size = 0;
  
  return 0;
}

int
gfxSpvLoad(VkDevice l_dev, const char* filename, VkShaderModule* shader)
{
  
  FILE* file = fopen(filename, "rb");
  if(file == NULL) {
    printf("%s not found!", filename);
    return 1;
  }
  if(fseek(file, 0l, SEEK_END) != 0) {
    printf("failed to seek to end of file!");
    return 1;
  }
  size_t length = ftell(file);
  if (length == 0){
    printf("failed to get file size!");
    return 1;
  }

  char *spv_code = (char *)malloc(length * sizeof(char));
  rewind(file);
  fread(spv_code, length, 1, file);

  VkShaderModuleCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = length,
    .pCode = (const uint32_t*)spv_code
  };

  if (vkCreateShaderModule(l_dev, &create_info, NULL, shader)
     != VK_SUCCESS) {
    return 1;
  }

  fclose(file);
  free(spv_code);

  return 0;
}

int gfxPipelineInit(GfxContext* vk){

  VkPushConstantRange push_constant = {
    .offset = 0,
    .size = sizeof(GfxPushConstant),
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
  };
  
  VkPipelineLayoutCreateInfo pipeline_layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &vk->texture_descriptors_layout,
    .pPushConstantRanges = &push_constant,
    .pushConstantRangeCount = 1,
  };

  if(vkCreatePipelineLayout(vk->ldev, &pipeline_layout_info, NULL, &vk->pipeline_layout) != VK_SUCCESS)
    {
      printf("!failed to create pipeline layout!\n");
      return 1;
    }

  VkShaderModule vert_shader;
  gfxSpvLoad(vk->ldev, "shaders/vert.spv", &vert_shader);
  VkShaderModule frag_shader;
  gfxSpvLoad(vk->ldev, "shaders/frag.spv", &frag_shader);
  
  VkPipelineShaderStageCreateInfo vert_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = vert_shader,
    .pName = "main",
    .pSpecializationInfo = NULL,
  };

  VkPipelineShaderStageCreateInfo frag_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = frag_shader,
    .pName = "main",
    .pSpecializationInfo = NULL,
  };

  VkPipelineShaderStageCreateInfo shader_stages[2] =
    {vert_info, frag_info};

  VkPipelineDepthStencilStateCreateInfo depth_stencil = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_TRUE,
    .depthWriteEnable = VK_TRUE,
    .depthCompareOp = VK_COMPARE_OP_LESS,
    .depthBoundsTestEnable = VK_FALSE,
    .minDepthBounds = 0.0f,
    .maxDepthBounds = 1.0f,
    .stencilTestEnable = VK_FALSE,
    .front = { 0 },
    .back = { 0 },
  }; // Model Specific
  
  VkDynamicState dynamic_states[2] =
    { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  
  VkPipelineDynamicStateCreateInfo dynamic_state_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]),
    .pDynamicStates = dynamic_states
  };

  // Vertex Buffer Creation
  int attribute_count = 2;
  VkVertexInputAttributeDescription attribute_descriptions[attribute_count];

  /* some instance attributes are two UINT16 packed
   * into a 32bit number */
  // Position PACKED
  attribute_descriptions[0].binding = 0;
  attribute_descriptions[0].location = 0;
  attribute_descriptions[0].format = VK_FORMAT_R32_UINT;
  attribute_descriptions[0].offset =
    offsetof(GfxGlyph, pos);

  // Glyph Encoding ( x, y ) PACKED
  attribute_descriptions[1].binding = 0;
  attribute_descriptions[1].location = 1;
  attribute_descriptions[1].format = VK_FORMAT_R32_UINT;
  attribute_descriptions[1].offset =
    offsetof(GfxGlyph, unicode_atlas_and_colors);

  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = sizeof(GfxGlyph),
    .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
  };
  
  VkPipelineVertexInputStateCreateInfo vertex_input_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 1,
    .vertexAttributeDescriptionCount = attribute_count,
    .pVertexBindingDescriptions = &binding_description,
    .pVertexAttributeDescriptions = attribute_descriptions,
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,
  };

  VkViewport viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = vk->extent.width,
    .height = vk->extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };

  VkRect2D scissor  = {
    .offset = { 0, 0},
    .extent = vk->extent,
  };

  VkPipelineViewportStateCreateInfo viewport_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor,
  };

  VkPipelineRasterizationStateCreateInfo rasterization_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .lineWidth = 1.0f,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp = 0.0f,
    .depthBiasSlopeFactor = 0.0f,
  };

  VkPipelineMultisampleStateCreateInfo multisample_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .sampleShadingEnable = VK_FALSE,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .minSampleShading = 1.0f,
    .pSampleMask = NULL,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE,
  };

  VkPipelineColorBlendAttachmentState color_blend_attachment = {
    .colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
    | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp = VK_BLEND_OP_ADD, // Optional
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, // Optional
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // Optional
    .alphaBlendOp = VK_BLEND_OP_ADD, // Optional
  };

  VkPipelineColorBlendStateCreateInfo color_blend_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment,
    .blendConstants[0] = 0.0f,
    .blendConstants[1] = 0.0f,
    .blendConstants[2] = 0.0f,
    .blendConstants[3] = 0.0f,
  };

  VkFormat cfg_format_val = cfg_format;
  VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &cfg_format_val,
  };

  VkGraphicsPipelineCreateInfo pipeline_info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = 2,
    .pStages = shader_stages,
    .pVertexInputState = &vertex_input_info,
    .pInputAssemblyState = &input_assembly_info,
    .pViewportState = &viewport_info,
    .pRasterizationState = &rasterization_info,
    .pMultisampleState = &multisample_info,
    .pDepthStencilState = &depth_stencil,
    .pColorBlendState = &color_blend_info,
    .pDynamicState = &dynamic_state_info,
    .layout = vk->pipeline_layout,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = -1,
    .pNext = &pipeline_rendering_create_info,
  };

  if(vkCreateGraphicsPipelines
     (vk->ldev, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &vk->pipeline) != VK_SUCCESS) {
    printf("!failed to create graphics pipeline!\n");
  }
    
  vkDestroyShaderModule(vk->ldev, frag_shader, NULL);
  vkDestroyShaderModule(vk->ldev, vert_shader, NULL);
  
  return 0;
}
