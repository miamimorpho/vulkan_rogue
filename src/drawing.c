#include "macros.h"
#include "drawing.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/* could be optimised, fast enough if results cached */
uint32_t getUnicodeUV(GfxTileset tileset, uint32_t unicode){
  for(uint32_t i = 0; i < tileset.glyph_c; i++){
    if(tileset.encoding[i] == unicode){
      return i;
    }
  }
  return 0;
}

uint32_t pack16into32(uint16_t a, uint16_t b){
  return(uint32_t)a << 16 | (uint32_t)b;
}

int _gfxAddCh(GfxGlobal* global, uint16_t x, uint16_t y,
	      uint16_t encoding, uint16_t texture_index,
	      uint16_t fg, uint16_t bg){

  GfxTileset texture = global->textures[texture_index];
  if(texture.image.handle == NULL){
    return 1;
  }

  if(x >= global->tile_buffer_w || x < 0) return 1;
  if(y >= global->tile_buffer_h || y < 0) return 1;

  TileDrawInstance src = {
    .pos = pack16into32(x, y),
    .tex_encoding = getUnicodeUV(global->textures[texture_index], encoding),
    .tex_index_and_width =
    pack16into32(texture_index, texture.image_w / ASCII_TILE_SIZE),

    .color_indices = pack16into32(fg, bg)
  };
  global->tile_buffer[(y * global->tile_buffer_w) + x] = src;
  
  return 0;
}

int _gfxAddString(GfxContext gfx, GfxGlobal* global,
		   uint16_t x, uint16_t y,
		   const char* str,
		   uint16_t fg, uint16_t bg){
  int i = 0;
  int start_x = 0;
  while(str[i] != '\0') {
    if(str[i] == '\n'){
      y++;
      x = start_x;
    }else{
      int err = _gfxAddCh(global, x++, y,
			  str[i], ASCII_TEXTURE_INDEX,
			  fg, bg);
      if(err != 0){
	return 1;
      }
    }
    i++;
  }
  return 0;
}

/* pre-bake command buffers for each swapchain image,
 * need to re re-record on window re-size
 */
int _gfxBakeCommandBuffers(GfxContext gfx, GfxGlobal global)
{
  for(unsigned int i = 0; i < gfx.swapchain_c; i++){
    
  VkCommandBuffer cmd_b = gfx.cmd_buffer[i];
  vkResetCommandBuffer(cmd_b, 0);

  
  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = 0,
    .pInheritanceInfo = NULL,
  };
  
  if(vkBeginCommandBuffer(cmd_b, &begin_info) != VK_SUCCESS){
    printf("!failed to begin recording command buffer!\n");
  }

  transitionImageLayout(cmd_b, gfx.swapchain_images[i],
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


  transitionImageLayout(cmd_b, gfx.swapchain_images[i],
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  
  VkClearValue clear_value;
  clear_value.color = 
    (VkClearColorValue){ { 0.0f, 0.0f, 0.0f, 1.0f } };
  
  VkRenderingAttachmentInfoKHR color_attachment_info = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
    .imageView = gfx.swapchain_views[i],
    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = clear_value,
  };
    
  VkRenderingInfoKHR render_info = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
    .renderArea.offset = {0, 0},
    .renderArea.extent = gfx.extent,
    .layerCount = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment_info,
  };
  pfn_vkCmdBeginRenderingKHR(cmd_b, &render_info);



    
/*
    VkRenderPassBeginInfo render_pass_info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = gfx.renderpass,
      .framebuffer = gfx.framebuffer[i],
      .renderArea.offset = {0, 0},
      .renderArea.extent = gfx.extent,
      .clearValueCount = 1,
      .pClearValues = clears,
    };
    
    vkCmdBeginRenderPass(cmd_b, &render_pass_info,
			 VK_SUBPASS_CONTENTS_INLINE);
*/
			 
    GfxPushConstant constants = {
      .screen_size_px = (vec2){ gfx.extent.width, gfx.extent.height },
    };
    vkCmdPushConstants(cmd_b, gfx.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GfxPushConstant), &constants);
    
    vkCmdBindDescriptorSets(cmd_b, VK_PIPELINE_BIND_POINT_GRAPHICS,
			    gfx.pipeline_layout, 0, 1,
			    &gfx.texture_descriptors, 0, NULL);
    
    VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = gfx.extent.width * ASCII_SCALE,
      .height = gfx.extent.height * ASCII_SCALE,
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
    
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd_b, 0, 1,
			   &global.tile_draw_instances.handle, offsets);    
    vkCmdDraw(cmd_b, 6,
	      global.tile_buffer_w * global.tile_buffer_h, 0, 0);
    
    // draw end
    pfn_vkCmdEndRenderingKHR(cmd_b);

    if(vkEndCommandBuffer(cmd_b) != VK_SUCCESS) {
      printf("!failed to record command buffer!");
      return 1;
    }
  }
  return 0;
}

int _gfxRenderFrame(GfxContext gfx, GfxGlobal* global){

  // waits on last VkQueueSubmit to be done
  VkResult fence_result;
  fence_result = vkWaitForFences(gfx.ldev, 1,
		    &gfx.fence[global->frame_x],
		    VK_TRUE, UINT32_MAX);
  if(fence_result == VK_TIMEOUT){
    printf("FATAL: VkWaitForFences timed out\n");
    abort();
  }
  vkResetFences(gfx.ldev, 1, &gfx.fence[global->frame_x]);
  
  VkSemaphore* image_available = &gfx.image_available[global->frame_x];  
  VkResult result = VK_TIMEOUT;
  result = vkAcquireNextImageKHR(gfx.ldev, gfx.swapchain, UINT32_MAX,
				   *image_available,
				   VK_NULL_HANDLE,
				   &global->swapchain_x);
  if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    gfxRecreateSwapchain();
    vkDestroySemaphore(gfx.ldev, *image_available, NULL);
    VkSemaphoreCreateInfo semaphore_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VK_CHECK(vkCreateSemaphore(gfx.ldev, &semaphore_info, NULL, image_available));
    return 1;
  }

  // Copy draw commands to GPU
  const int src_size = global->tile_buffer_w * global->tile_buffer_h * sizeof(TileDrawInstance);
  /* Profiles at 0.02ms */
  // pull this out of presentation.
  gfxBufferAppend(gfx.allocator, &global->tile_draw_instances, global->tile_buffer, src_size);
  global->tile_draw_instances.used_size = 0;

  VkPipelineStageFlags wait_stages[] =
    {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
 
  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = image_available,
    .pWaitDstStageMask = wait_stages,
    .commandBufferCount = 1,
    .pCommandBuffers = &gfx.cmd_buffer[global->swapchain_x],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &gfx.render_finished[global->frame_x],
  };

  // waits on VkAcquireImageKHR to signal an image is available
  VK_CHECK(vkQueueSubmit(gfx.queue, 1, &submit_info, gfx.fence[global->frame_x]));

  VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &gfx.render_finished[global->frame_x],
    .swapchainCount = 1,
    .pSwapchains = &gfx.swapchain,
    .pImageIndices = &global->swapchain_x,
    .pResults = NULL
  };

  // waits on vkQueueSubmit to signal its finished rendering
  VkResult present_result = vkQueuePresentKHR(gfx.queue, &present_info);
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

  global->frame_x = (global->frame_x + 1) % gfx.frame_c;

  return 0;
}

int gfxPipelineInit(GfxContext* gfx){

  VkPushConstantRange push_constant = {
    .offset = 0,
    .size = sizeof(GfxPushConstant),
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
  };
  
  VkPipelineLayoutCreateInfo pipeline_layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &gfx->texture_descriptors_layout,
    .pPushConstantRanges = &push_constant,
    .pushConstantRangeCount = 1,
  };

  if(vkCreatePipelineLayout(gfx->ldev, &pipeline_layout_info, NULL, &gfx->pipeline_layout) != VK_SUCCESS)
    {
      printf("!failed to create pipeline layout!\n");
      return 1;
    }

  VkShaderModule vert_shader;
  gfxSpvLoad(gfx->ldev, "shaders/vert.spv", &vert_shader);
  VkShaderModule frag_shader;
  gfxSpvLoad(gfx->ldev, "shaders/frag.spv", &frag_shader);
  
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
  int attribute_count = 4;
  VkVertexInputAttributeDescription attribute_descriptions[attribute_count];

  /* some instance attributes are two UINT16 packed
   * into a 32bit number */
  // Position PACKED
  attribute_descriptions[0].binding = 0;
  attribute_descriptions[0].location = 0;
  attribute_descriptions[0].format = VK_FORMAT_R32_UINT;
  attribute_descriptions[0].offset =
    offsetof(TileDrawInstance, pos);

  // Glyph Encoding ( x, y ) PACKED
  attribute_descriptions[1].binding = 0;
  attribute_descriptions[1].location = 1;
  attribute_descriptions[1].format = VK_FORMAT_R32_UINT;
  attribute_descriptions[1].offset =
    offsetof(TileDrawInstance, tex_encoding);

  // Texture Index + size PACKED
  attribute_descriptions[2].binding = 0;
  attribute_descriptions[2].location = 2;
  attribute_descriptions[2].format = VK_FORMAT_R32_UINT;
  attribute_descriptions[2].offset =
    offsetof(TileDrawInstance, tex_index_and_width);

  // Colors, two 16 bit numbers packed into a 32bit
  attribute_descriptions[3].binding = 0;
  attribute_descriptions[3].location = 3;
  attribute_descriptions[3].format = VK_FORMAT_R32_UINT;
  attribute_descriptions[3].offset =
    offsetof(TileDrawInstance, color_indices);

  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = sizeof(TileDrawInstance),
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
    .width = gfx->extent.width,
    .height = gfx->extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };

  VkRect2D scissor  = {
    .offset = { 0, 0},
    .extent = gfx->extent,
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
    .layout = gfx->pipeline_layout,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = -1,
    .pNext = &pipeline_rendering_create_info,
  };

  if(vkCreateGraphicsPipelines
     (gfx->ldev, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &gfx->pipeline) != VK_SUCCESS) {
    printf("!failed to create graphics pipeline!\n");
  }
    
  vkDestroyShaderModule(gfx->ldev, frag_shader, NULL);
  vkDestroyShaderModule(gfx->ldev, vert_shader, NULL);
  
  return 0;
}
