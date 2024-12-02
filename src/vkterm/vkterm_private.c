#include "vkterm_private.h"
#include "macros.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

PFN_vkCmdBeginRenderingKHR pfn_vkCmdBeginRenderingKHR;
PFN_vkCmdEndRenderingKHR pfn_vkCmdEndRenderingKHR;

VkResult init_vkCmdBeginRenderingKHR(VkDevice device) {
    pfn_vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)
      vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR" );    
    if (pfn_vkCmdBeginRenderingKHR == NULL) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    
    pfn_vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)
      vkGetDeviceProcAddr( device, "vkCmdEndRenderingKHR"
    );    
    if (pfn_vkCmdBeginRenderingKHR == NULL) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    
    return VK_SUCCESS;
}

void gfxGetScreenDimensions(GfxGlobal* gfx, int* dst_width, int* dst_height){
  *dst_width = gfx->width_in_tiles;
  *dst_height = gfx->height_in_tiles;
}

VkCommandBuffer gfxCmdSingleBegin(GfxContext vk)
{
  VkCommandBufferAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandPool = vk.cmd_pool,
    .commandBufferCount = 1,
  };

  VkCommandBuffer command_buffer;
  vkAllocateCommandBuffers(vk.ldev, &alloc_info, &command_buffer);

  VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  vkBeginCommandBuffer(command_buffer, &begin_info);

  return command_buffer;
}

int gfxCmdSingleEnd(GfxContext vk, VkCommandBuffer cmd_buffer)
{
  vkEndCommandBuffer(cmd_buffer);

  VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd_buffer,
  };

  if(vkQueueSubmit(vk.queue, 1, &submit_info, VK_NULL_HANDLE)
     != VK_SUCCESS) return 1;
  
  vkQueueWaitIdle(vk.queue);
  vkFreeCommandBuffers(vk.ldev, vk.cmd_pool, 1, &cmd_buffer);
  return 0;
}

int gfxBufferAppend(VmaAllocator allocator, GfxBuffer* dest,
		    const void* src, VkDeviceSize src_size)
{
  VmaAllocationInfo dest_info;
  vmaGetAllocationInfo(allocator, dest->allocation, &dest_info);
  
  if(dest->used_size + src_size > dest_info.size){
    //printf("fail\n");
    return 1;
  }
  
  vmaCopyMemoryToAllocation(allocator, src,
			    dest->allocation, dest->used_size,
			    src_size);
  dest->used_size += src_size;
  return 0;
}

size_t gfxBufferCapacity(VmaAllocator allocator, GfxBuffer buffer){
  VmaAllocationInfo info;
  vmaGetAllocationInfo(allocator,
		       buffer.allocation,
		       &info);
  return info.size;
}

GfxBuffer* gfxBufferNext(VmaAllocator allocator, GfxBuffer* first)
{

  VmaAllocationInfo first_info;
  vmaGetAllocationInfo(allocator, first->allocation,
		       &first_info);

  GfxBuffer* second = (GfxBuffer*)first_info.pUserData;
  return second;
}

int gfxBufferCreate(VmaAllocator allocator, VkBufferUsageFlags usage, VkDeviceSize size, GfxBuffer* dest)
{
  if(DEBUG_BUFFER){
    printf("+creating buffer %p\n", (void*)dest);
  }

  VkBufferCreateInfo buffer_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = usage
  };
  VmaAllocationCreateInfo vma_alloc_info = {
    .usage = VMA_MEMORY_USAGE_AUTO,
    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
  };

  VK_CHECK(vmaCreateBuffer(allocator, &buffer_info,
			   &vma_alloc_info,
			   &dest->handle,
			   &dest->allocation, NULL));
  dest->used_size = 0;
    
  return 0;
}

int
gfxBufferDestroy(VmaAllocator allocator, GfxBuffer* b)
{
  if (allocator == NULL || b == NULL) {
    // Log error or handle appropriately
    return 1;
  }
  
  VmaAllocationInfo b_info;
  vmaGetAllocationInfo(allocator, b->allocation, &b_info);
  
  if(b_info.pUserData != NULL){
    GfxBuffer* next_buffer = gfxBufferNext(allocator, b);
    gfxBufferDestroy(allocator, next_buffer);
    free(next_buffer);
  }
  
  vmaDestroyBuffer(allocator, b->handle, b->allocation);
  return 0;
}

int
gfxImageAlloc(VmaAllocator allocator, GfxImage* image,
	      VkImageUsageFlags usage, VkFormat format,
	      uint32_t width, uint32_t height)
{

  /* Allocate VkImage memory */
  VkImageCreateInfo image_info = {
    .sType= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .extent.width = width,
    .extent.height = height,
    .extent.depth = 1,
    .mipLevels = 1,
    .arrayLayers = 1,
    .format = format,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .samples = VK_SAMPLE_COUNT_1_BIT,
  };

  VmaAllocationCreateInfo alloc_create_info = {
    .usage = VMA_MEMORY_USAGE_AUTO,
    .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
    .priority = 1.0f,
  };
  
  VK_CHECK(vmaCreateImage(allocator, &image_info, &alloc_create_info,
			  &image->handle, &image->allocation, NULL));
  return 0;
}

int
gfxImageViewCreate(VkDevice l_dev, VkImage image,
		  VkImageView *view, VkFormat format,
		  VkImageAspectFlags aspect_flags)
{
  VkImageViewCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = image,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format = format,
    .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
    .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
    .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
    .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
    .subresourceRange.aspectMask = aspect_flags,
    .subresourceRange.baseMipLevel = 0,
    .subresourceRange.levelCount = 1,
    .subresourceRange.baseArrayLayer = 0,
    .subresourceRange.layerCount = 1,
  };
  VK_CHECK(vkCreateImageView(l_dev, &create_info, NULL, view));
  return 0;
}

int
copyBufferToImage(GfxContext vk, VkBuffer buffer, VkImage image,
		  uint32_t width, uint32_t height)
{
  VkCommandBuffer command = gfxCmdSingleBegin(vk);
  
  VkBufferImageCopy region = {
    .bufferOffset = 0,
    .bufferRowLength = 0,
    .bufferImageHeight = 0,

    .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .imageSubresource.mipLevel = 0,
    .imageSubresource.baseArrayLayer = 0,
    .imageSubresource.layerCount = 1,

    .imageOffset = {0, 0, 0},
    .imageExtent  = {width, height, 1},
  };

  vkCmdCopyBufferToImage(command, buffer, image,
			 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			 1, &region);
  
  return gfxCmdSingleEnd(vk, command);
}

int
transitionImageLayout(VkCommandBuffer commands, VkImage image,
		      VkImageLayout old_layout, VkImageLayout new_layout){

  VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .oldLayout = old_layout,
    .newLayout = new_layout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = image,
    .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .subresourceRange.baseMipLevel = 0,
    .subresourceRange.levelCount = 1,
    .subresourceRange.baseArrayLayer = 0,
    .subresourceRange.layerCount = 1,
    .srcAccessMask = 0,
    .dstAccessMask = 0,
  };

  VkPipelineStageFlags source_stage, destination_stage;

  int supported_transition = 0;
  if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED
     && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    supported_transition = 1;
  }
  if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
     && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    supported_transition = 1;
  }

  if(new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL){
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    supported_transition = 1;
  }
  if(old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL){
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    destination_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    supported_transition = 1;
  }
    
  if( supported_transition == 0){
    printf("unsupported layout transition\n");
    return 1;
  }
  vkCmdPipelineBarrier(commands,
		       source_stage, destination_stage,
		       0,
		       0, NULL,
		       0, NULL,
		       1, &barrier);
  return 0;
}

void
gfxImageDestroy(VmaAllocator allocator, GfxImage image)
{
  VmaAllocatorInfo alloc_info;
  vmaGetAllocatorInfo(allocator, &alloc_info);
  
  vkDestroySampler(alloc_info.device, image.sampler, NULL);
  vkDestroyImageView(alloc_info.device, image.view, NULL);
  vmaDestroyImage(allocator, image.handle, image.allocation);
}

int gfxAllocatorInit(GfxContext* gfx) {
  
  VmaAllocatorCreateInfo allocator_info = {
    .physicalDevice = gfx->pdev,
    .device = gfx->ldev,
    .instance = gfx->instance,
  };
  vmaCreateAllocator(&allocator_info, &gfx->allocator);
  return 0;
}

int gfxGlfwInit(GfxContext* gfx){
  
  if(!glfwInit()){
    printf("glfw init failure\n");
    return 1;
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  int width = ASCII_SCREEN_WIDTH * ASCII_TILE_SIZE * ASCII_SCALE;
  int height = ASCII_SCREEN_HEIGHT * ASCII_TILE_SIZE * ASCII_SCALE;
  gfx->window = glfwCreateWindow(width, height, "Vulkan/GLFW", NULL, NULL);

  if(gfx->window == NULL){
    printf("glfw init failed\n");
    return 1;
  }
  
  return 0;
}

int gfxInstanceInit(GfxContext* gfx){
  uint32_t vk_version;
  vkEnumerateInstanceVersion(&vk_version);

  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "Cars",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "Naomi Engine",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = vk_version,//VK_API_VERSION_1_3,
  };

  uint32_t ext_c;
  const char** ext = glfwGetRequiredInstanceExtensions(&ext_c);

  uint32_t vk_ext_c = 0;
  vkEnumerateInstanceExtensionProperties(NULL, &vk_ext_c, NULL);
  VkExtensionProperties vk_ext[vk_ext_c+1];
  vkEnumerateInstanceExtensionProperties(NULL, &vk_ext_c, vk_ext);

  int ext_found;
  for (uint32_t g = 0; g < ext_c; g++){
    ext_found = 0;
    for (uint32_t i = 0; i < vk_ext_c; i++){
      if (strcmp(ext[g], vk_ext[i].extensionName) == 0){
	ext_found = 1;
      }
    }
    if(!ext_found){
      printf("!extension not found! %s\n", ext[g]);
      return 1;
    }
  }

  VkInstanceCreateInfo instance_create_info = {
    .sType= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info,
    .enabledExtensionCount = ext_c,
    .ppEnabledExtensionNames = ext,
    .enabledLayerCount = 0,
  };

  /* Enable validation_layers debugging */
  instance_create_info.enabledLayerCount = 0;
  if(DEBUG_LAYERS != 0){
    const char *explicit_layers[] = { "VK_LAYER_KHRONOS_validation"};
    uint32_t debug_layer_c;
    vkEnumerateInstanceLayerProperties(&debug_layer_c, NULL);
    VkLayerProperties debug_layers[debug_layer_c];
    vkEnumerateInstanceLayerProperties(&debug_layer_c, debug_layers);
    
    instance_create_info.enabledLayerCount = 1;
    instance_create_info.ppEnabledLayerNames = explicit_layers;
  }
    
  VK_CHECK(vkCreateInstance(&instance_create_info, NULL, &gfx->instance));

  return 0;
}

int gfxPhysicalDeviceInit(GfxContext* gfx){
  uint32_t dev_c = 0;
  vkEnumeratePhysicalDevices(gfx->instance, &dev_c, NULL);
  if(dev_c == 0){
    printf("FATAL no devices found\n");
    return 1;
  }
  if(dev_c > 1 ){
    printf("Warning: multiple graphics devices found, dont have the logic to choose\n");
  }
  VkPhysicalDevice devs[dev_c];
  vkEnumeratePhysicalDevices(gfx->instance, &dev_c, devs);
  for(uint32_t i = 0; i < dev_c; i++){
    VkPhysicalDeviceProperties dev_properties;
    vkGetPhysicalDeviceProperties(devs[i], &dev_properties);
    printf("DEBUG %s\n", dev_properties.deviceName);
  }

  gfx->pdev = devs[0];
  
  VkPhysicalDeviceFeatures dev_features;
  vkGetPhysicalDeviceFeatures(gfx->pdev, &dev_features);  
  if(!dev_features.geometryShader){
    printf("FATAL: driver missing geometry shader\n");
    return 1;
  }
  if(!dev_features.samplerAnisotropy){
     printf("FATAL: driver missing sampler anisotropy\n");
     return 1;
  }
  if(!dev_features.multiDrawIndirect){
     printf("FATAL: driving missing indirect drawing\n");
     return 1;
  }
  
  return 0;
}

int gfxQueueIndex(GfxContext* gfx){
  uint32_t q_fam_c;
  vkGetPhysicalDeviceQueueFamilyProperties(gfx->pdev, &q_fam_c, NULL);
  VkQueueFamilyProperties q_fam[q_fam_c];
  vkGetPhysicalDeviceQueueFamilyProperties(gfx->pdev, &q_fam_c, q_fam);

  uint32_t queue_index = -1;
  for(uint32_t i = 0; i < q_fam_c; i++){
    if(q_fam[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
      queue_index = i;
      break;
    }
  }
  if(queue_index < 0){
    printf("FATAL valid queue family not found\n");
  }

  return queue_index;
}

int gfxLogicalDeviceInit(GfxContext* gfx){

  /* Requested Extensions */
  const char *dev_ext_names[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    "VK_EXT_descriptor_indexing"
  };
  const unsigned int dev_ext_c = 3;

  /* Available Extensions */
  uint32_t avail_ext_c;
  vkEnumerateDeviceExtensionProperties(gfx->pdev, NULL, &avail_ext_c, NULL);
  VkExtensionProperties avail_ext[avail_ext_c];
  vkEnumerateDeviceExtensionProperties(gfx->pdev, NULL, &avail_ext_c, avail_ext);

  /* Check Extension Availability */
  int layer_found;
  for(uint32_t i = 0; i < dev_ext_c; i++){
    layer_found = 0;
    for(uint32_t a = 0; a < avail_ext_c; a++){
      if(strcmp(dev_ext_names[i], avail_ext[a].extensionName) == 0){
	layer_found = 1;
      }
    }
    if(!layer_found){
      printf("FATAL: driver missing %s\n", dev_ext_names[i]);
      return 1;
    }
  }
 
  int queue_index = gfxQueueIndex(gfx);
  float priority = 1.0f;
  VkDeviceQueueCreateInfo q_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = queue_index,
    .queueCount = 1,
    .pQueuePriorities = &priority,
  };

  /* Enable Descriptor Indexing */
  VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
  };

  VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
    .dynamicRendering = VK_TRUE,
    .pNext = &descriptor_indexing_features
  };
    
  VkPhysicalDeviceFeatures2 dev_features2 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    .pNext = &dynamic_rendering_feature,
  };

  vkGetPhysicalDeviceFeatures2(gfx->pdev, &dev_features2 );
 
  if(!descriptor_indexing_features.descriptorBindingPartiallyBound){
    printf("FATAL: driver missing 'descriptor Binding Partially Bound'\n");
    exit(1);
  }
  if(!descriptor_indexing_features.runtimeDescriptorArray){
    printf("FATAL: driver missing 'runtime descriptor array'\n");
    exit(1);
  }
  if(!dynamic_rendering_feature.dynamicRendering){
    printf("FATAL: driver missing 'dynamic rendering'\n");
    exit(1);
  }
  VkDeviceCreateInfo dev_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pQueueCreateInfos = &q_create_info,
    .queueCreateInfoCount = 1,
    //.pEnabledFeatures = &dev_features,
    .enabledExtensionCount = dev_ext_c,
    .ppEnabledExtensionNames = dev_ext_names,
    .enabledLayerCount = 0,
    .pNext = &dev_features2,
  };

  if(vkCreateDevice(gfx->pdev, &dev_create_info, NULL,
		    &gfx->ldev)
     != VK_SUCCESS) {
    printf("FATAL failed to create logical device!\n");
    return 2;
  }

  vkGetDeviceQueue(gfx->ldev, queue_index, 0,
		   &gfx->queue);
  
  return 0;
}

int gfxCmdPoolInit(GfxContext* gfx){
   VkCommandPoolCreateInfo command_pool_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = gfxQueueIndex(gfx),
  };

  if(vkCreateCommandPool(gfx->ldev, &command_pool_info, NULL, &gfx->cmd_pool)
     != VK_SUCCESS){
    printf("!failed to create command pool!");
    return 1;
  }
  return 0;
}

int gfxSurfaceInit(GfxContext* gfx){
  
  if(glfwCreateWindowSurface(gfx->instance, gfx->window,
			     NULL, &gfx->surface)){
    printf("failed to create glfw surface");
    glfwDestroyWindow(gfx->window);
    glfwTerminate();
    return 1;
  }
  
  /* Test for queue presentation support */
  VkBool32 present_support = VK_FALSE;
  vkGetPhysicalDeviceSurfaceSupportKHR
    (gfx->pdev, gfxQueueIndex(gfx),
     gfx->surface, &present_support);
  if(present_support == VK_FALSE){
    printf("!no device presentation support!\n");
    return -2;
  }
  
  /* Find colour format */
  uint32_t format_c;
  vkGetPhysicalDeviceSurfaceFormatsKHR
    (gfx->pdev, gfx->surface, &format_c, NULL);
  VkSurfaceFormatKHR formats[format_c];
  vkGetPhysicalDeviceSurfaceFormatsKHR(gfx->pdev, gfx->surface,
				       &format_c, formats);

  VkBool32 supported = VK_FALSE;
  for (uint32_t i = 0; i < format_c; i++) {
    if(formats[i].format == cfg_format
       && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      supported = VK_TRUE;
  }
  if (supported == VK_FALSE) {
    printf("!colour format not supported!\n");
    return 1;
  }
  
  /* Presentation mode */
  uint32_t mode_c;
  vkGetPhysicalDeviceSurfacePresentModesKHR(gfx->pdev, gfx->surface, &mode_c, NULL);
  VkPresentModeKHR modes[mode_c];
  vkGetPhysicalDeviceSurfacePresentModesKHR(gfx->pdev, gfx->surface, &mode_c, modes);

  supported = VK_FALSE;
  for (uint32_t i = 0; i < mode_c; i++) {
    if (modes[i]== VK_PRESENT_MODE_FIFO_KHR)
      supported = VK_TRUE;
  }
  if (supported == VK_FALSE) {
    printf("!presentation mode not supported!\n");
    return 2;
  }

  return 0;
}

int gfxSwapchainInit(GfxContext* gfx){
  glfwGetWindowSize(gfx->window,
		    (int*)&gfx->extent.width,
		    (int*)&gfx->extent.height);
  
  VkSurfaceCapabilitiesKHR capable;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR
    (gfx->pdev, gfx->surface, &capable);

  if (capable.currentExtent.width != gfx->extent.width
     || capable.currentExtent.height != gfx->extent.width) {
    gfx->extent.width =
      gfx->extent.width > capable.maxImageExtent.width ?
      capable.maxImageExtent.width : gfx->extent.width;

    gfx->extent.width =
      gfx->extent.width < capable.minImageExtent.width ?
      capable.minImageExtent.width : gfx->extent.width;
    
    gfx->extent.height =
      gfx->extent.height > capable.maxImageExtent.height ?
      capable.maxImageExtent.height : gfx->extent.height;

    gfx->extent.height =
      gfx->extent.height < capable.minImageExtent.height ?
      capable.minImageExtent.height : gfx->extent.height;
  }
   
  uint32_t max_swapchain_c = capable.minImageCount + 1;
  if(capable.maxImageCount > 0 && max_swapchain_c > capable.maxImageCount) {
    max_swapchain_c = capable.maxImageCount;
  }
  gfx->frame_c = DRAW_BUFFER_COUNT;
  
  VkSwapchainCreateInfoKHR swapchain_create_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = gfx->surface,
    .minImageCount = max_swapchain_c,
    .imageFormat = cfg_format,
    .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    .imageExtent = gfx->extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    /* TODO: multiple queue indices ( current [0][0] ) */
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = NULL,
    .preTransform = capable.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode =  VK_PRESENT_MODE_FIFO_KHR,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE,
  };

  if (vkCreateSwapchainKHR(gfx->ldev, &swapchain_create_info, NULL,
			   &gfx->swapchain)
     != VK_SUCCESS){
    printf("!failed to create swap chain!\n");
    return 1;
  }

  /* Create Image Views */
  vkGetSwapchainImagesKHR
    (gfx->ldev, gfx->swapchain, &gfx->swapchain_c, NULL);
  gfx->swapchain_images = malloc(gfx->swapchain_c * sizeof(VkImage));
  gfx->swapchain_views = malloc(gfx->swapchain_c * sizeof(VkImageView));

  vkGetSwapchainImagesKHR
    (gfx->ldev, gfx->swapchain,
     &gfx->swapchain_c, gfx->swapchain_images);

  for (uint32_t i = 0; i < gfx->swapchain_c; i++) {
    gfxImageViewCreate(gfx->ldev, gfx->swapchain_images[i],
		      &gfx->swapchain_views[i], cfg_format,
		      VK_IMAGE_ASPECT_COLOR_BIT);
  }

  return 0;
}

int gfxRecreateSwapchain(GfxContext* gfx){
  
  vkDeviceWaitIdle(gfx->ldev);
  
  _gfxSwapchainDestroy(*gfx);
  
  gfxSwapchainInit(gfx);

  vkDeviceWaitIdle(gfx->ldev);
  
  return 0;
}

int gfxDescriptorsPool(GfxContext* gfx){
  
  VkDescriptorPoolSize pool_sizes[2];
  pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_sizes[0].descriptorCount = MAX_SAMPLERS;
  
  pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  pool_sizes[1].descriptorCount = gfx->frame_c;
 
  VkDescriptorPoolCreateInfo descriptor_pool_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .poolSizeCount = 2,
    .pPoolSizes = pool_sizes,
    .maxSets = MAX_SAMPLERS + gfx->frame_c,
    .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
  };

  if(vkCreateDescriptorPool(gfx->ldev, &descriptor_pool_info, NULL, &gfx->descriptor_pool) != VK_SUCCESS) {
   return 1;
  }

  return 0;
}

int gfxSyncInit(GfxContext* gfx){
  
  VkSemaphoreCreateInfo semaphore_info = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };

  VkFenceCreateInfo fence_info = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  
  gfx->image_available = (VkSemaphore*)malloc( gfx->frame_c * sizeof(VkSemaphore));
  gfx->render_finished = (VkSemaphore*)malloc( gfx->frame_c * sizeof(VkSemaphore));
  gfx->fence = (VkFence*)malloc( gfx->frame_c * sizeof(VkFence));
  
  for(unsigned int i = 0; i < gfx->frame_c; i++){
  
    if(vkCreateSemaphore(gfx->ldev, &semaphore_info,
			 NULL, &gfx->image_available[i]) != VK_SUCCESS ||
       vkCreateSemaphore(gfx->ldev, &semaphore_info,
			 NULL, &gfx->render_finished[i]) != VK_SUCCESS ||
       vkCreateFence(gfx->ldev, &fence_info,
		     NULL, &gfx->fence[i]) != VK_SUCCESS ){
      printf("!failed to create sync objects!\n");
      return 1;
    }
  }
  return 0;
}

/* each command buffer is 'baked' with the drawing commands so
 * we need to specify a swapchain image index per buffer
 */
int gfxCmdBuffersInit(GfxContext* gfx)
{
  gfx->cmd_buffer = (VkCommandBuffer*)malloc
    (gfx->swapchain_c * sizeof(VkCommandBuffer) );
  for(unsigned int i = 0; i < gfx->swapchain_c; i++){
    gfx->cmd_buffer[i] = VK_NULL_HANDLE;
  }
  
  VkCommandBufferAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = gfx->cmd_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = gfx->swapchain_c,
  };
  
  if(vkAllocateCommandBuffers
     (gfx->ldev, &allocate_info, gfx->cmd_buffer)
     != VK_SUCCESS) return 1;

  return 0;
}

int gfxTextureDescriptorsInit(GfxContext* gfx){
  
  VkDescriptorSetLayoutBinding binding = {
    .binding = 0,
    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    .descriptorCount = MAX_SAMPLERS,
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
  };
  
  VkDescriptorBindingFlags bindless_flags =
    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT
    | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
    | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
  
  VkDescriptorSetLayoutBindingFlagsCreateInfo info2 = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
    .bindingCount = 1,
    .pBindingFlags = &bindless_flags,
  };
  
  VkDescriptorSetLayoutCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = 1,
    .pBindings = &binding,
    .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
    .pNext = &info2,
  };
  
  if(vkCreateDescriptorSetLayout(gfx->ldev, &info, NULL,
				 &gfx->texture_descriptors_layout) != VK_SUCCESS) return 1;

  uint32_t max_binding = MAX_SAMPLERS -1;
  VkDescriptorSetVariableDescriptorCountAllocateInfo count_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
    .descriptorSetCount = 1,
    .pDescriptorCounts = &max_binding,
  };
  
  VkDescriptorSetAllocateInfo alloc_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = gfx->descriptor_pool,
    .descriptorSetCount = 1,
    .pSetLayouts = &gfx->texture_descriptors_layout,
    .pNext = &count_info,
  };
  
  if(vkAllocateDescriptorSets(gfx->ldev, &alloc_info, &gfx->texture_descriptors)
     != VK_SUCCESS) return 1;
  return 0;
}



int _gfxSwapchainDestroy(GfxContext gfx){
  /*  for(uint32_t i = 0; i < gfx.swapchain_c; i++){
    vkDestroyFramebuffer(gfx.ldev, gfx.framebuffer[i], NULL); 
  }
  */
  for(uint32_t i = 0; i < gfx.swapchain_c; i++){
    vkDestroyImageView(gfx.ldev, gfx.swapchain_views[i], NULL);
  }
  vkDestroySwapchainKHR(gfx.ldev, gfx.swapchain, NULL);

  //free(gfx.framebuffer);
  free(gfx.swapchain_images);
  free(gfx.swapchain_views);

  return 0;
}

int _gfxConstFree(GfxContext gfx){

  // destroy sync objects
  for(unsigned int i = 0; i < gfx.frame_c; i++){
    vkDestroySemaphore(gfx.ldev, gfx.image_available[i], NULL);
    vkDestroySemaphore(gfx.ldev, gfx.render_finished[i], NULL);
    vkDestroyFence(gfx.ldev, gfx.fence[i], NULL);
  }

  free(gfx.image_available);
  free(gfx.render_finished);
  free(gfx.fence);
  free(gfx.cmd_buffer);

  vkDestroyCommandPool(gfx.ldev, gfx.cmd_pool, NULL);
  vkDestroyPipelineLayout(gfx.ldev, gfx.pipeline_layout, NULL);
  vkDestroyPipeline(gfx.ldev, gfx.pipeline, NULL);
  vkDestroyDescriptorSetLayout(gfx.ldev, gfx.texture_descriptors_layout, NULL);
  vkDestroyDescriptorPool(gfx.ldev, gfx.descriptor_pool, NULL);

  _gfxSwapchainDestroy(gfx);
    
  //vkDestroyRenderPass(gfx.ldev, gfx.renderpass, NULL);
  
  vmaDestroyAllocator(gfx.allocator);
  
  vkDestroyDevice(gfx.ldev, NULL);
  vkDestroySurfaceKHR(gfx.instance, gfx.surface, NULL);
  vkDestroyInstance(gfx.instance, NULL);

  glfwDestroyWindow(gfx.window);
  glfwTerminate();
  
  return 0;

}

// local
GfxGlobal* gfxScreenInit(void){

  GfxGlobal* gfx = malloc(sizeof(GfxGlobal));
  
  gfxGlfwInit(&gfx->vk);
  gfxInputInit(gfx->vk.window);
  gfxInstanceInit(&gfx->vk);
  gfxPhysicalDeviceInit(&gfx->vk);
  gfxQueueIndex(&gfx->vk);
  gfxLogicalDeviceInit(&gfx->vk);
  init_vkCmdBeginRenderingKHR(gfx->vk.ldev);

  gfxAllocatorInit(&gfx->vk);
  gfxCmdPoolInit(&gfx->vk);
  gfxSurfaceInit(&gfx->vk);
  gfxSwapchainInit(&gfx->vk);
  
  gfxDescriptorsPool(&gfx->vk);
  gfxSyncInit(&gfx->vk);
  gfxCmdBuffersInit(&gfx->vk);
  gfxTextureDescriptorsInit(&gfx->vk);
  gfxPipelineInit(&gfx->vk);

  gfx->width_in_tiles = gfx->vk.extent.width / (ASCII_TILE_SIZE * ASCII_SCALE);
  gfx->height_in_tiles = gfx->vk.extent.height / (ASCII_TILE_SIZE * ASCII_SCALE);
  
  gfxTexturesInit(&gfx->textures);

  gfx->swapchain_x = 0;
  gfx->frame_x = 0;
  
  // Create Scratch Buffers
  int tile_buffer_size = ASCII_SCREEN_WIDTH * ASCII_SCREEN_HEIGHT;
  gfxBufferCreate(gfx->vk.allocator,
		  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		  5 * tile_buffer_size * sizeof(GfxGlyph),
		  &gfx->gpu_glyph_cache);  
  gfx->caches = malloc(sizeof(GfxCache));
  gfx->cache_x = 0;
  gfx->cache_c = 0;
  gfxCacheChange(gfx, "main");
  // Create Draw Buffers
  gfxBufferCreate(gfx->vk.allocator,
		  VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		  5 * sizeof(VkDrawIndirectCommand), &gfx->indirect);
  
  return gfx;
}

int gfxScreenClose(GfxGlobal* gfx){

  GfxContext vk = gfx->vk;
  
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
  gfxBufferDestroy(vk.allocator, &gfx->gpu_glyph_cache);
  gfxBufferDestroy(vk.allocator, &gfx->indirect);
  
  // free asset data
  for(int i = 0; i < MAX_SAMPLERS; i++){
    if(gfx->textures[i].image.handle != VK_NULL_HANDLE){
      gfxImageDestroy(vk.allocator, gfx->textures[i].image);
    }
  }

  // free vulkan
  _gfxConstFree(gfx->vk);
  return 0;
}

