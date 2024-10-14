#include "vulkan_helper.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const char *DEV_EXT_NAMES[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, };
const unsigned int DEV_EXT_C = 1;
const VkFormat cfg_format = VK_FORMAT_B8G8R8A8_SRGB;

/* could be optimised, fast enough if results cached */
uint32_t getUnicodeUV(GfxTileset tileset, uint32_t unicode){
  for(uint32_t i = 0; i < tileset.glyph_c; i++){
    if(tileset.encoding[i] == unicode){
      return i;
    }
  }
  return 0;
}

int
gfxVertBufferCreate(GfxContext gfx, size_t estimated_size, GfxBuffer* dest_vertices)
{
  gfxBufferCreate(gfx.allocator,
		  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		  estimated_size * sizeof(vertex2),
		  dest_vertices);

  GfxBuffer* dest_indices = (GfxBuffer*)malloc(sizeof(GfxBuffer));
  gfxBufferCreate(gfx.allocator,
		  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		  estimated_size * sizeof(uint32_t) * 2,
		  dest_indices);

  vmaSetAllocationUserData(gfx.allocator,
			   dest_vertices->allocation,
			   dest_indices);

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
  if(DEBUG_BUFFER){
    printf("-destroying buffer %p\n", (void*)b);
  }
  
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
  const char *explicit_layers[] = { "VK_LAYER_KHRONOS_validation"};
  uint32_t debug_layer_c;
  vkEnumerateInstanceLayerProperties(&debug_layer_c, NULL);
  VkLayerProperties debug_layers[debug_layer_c];
  vkEnumerateInstanceLayerProperties(&debug_layer_c, debug_layers);
  
  instance_create_info.enabledLayerCount = 1;
  instance_create_info.ppEnabledLayerNames = explicit_layers;

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


 uint32_t avail_ext_c;
  vkEnumerateDeviceExtensionProperties(gfx->pdev, NULL, &avail_ext_c, NULL);
  VkExtensionProperties avail_ext[avail_ext_c];
  vkEnumerateDeviceExtensionProperties(gfx->pdev, NULL, &avail_ext_c, avail_ext);
  
  int layer_found;
  for(uint32_t i = 0; i < DEV_EXT_C; i++){
    layer_found = 0;
    for(uint32_t a = 0; a < avail_ext_c; a++){
      if(strcmp(DEV_EXT_NAMES[i], avail_ext[a].extensionName) == 0){
	layer_found = 1;
      }
    }
    if(!layer_found){
      printf("FATAL: driver missing %s\n", DEV_EXT_NAMES[i]);
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
  
  VkPhysicalDeviceFeatures2 dev_features2 = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    .pNext = &descriptor_indexing_features,
  };
  vkGetPhysicalDeviceFeatures2(gfx->pdev, &dev_features2 );
 
  if(!descriptor_indexing_features.descriptorBindingPartiallyBound)
    printf("FATAL: driver missing 'descriptor Binding Partially Bound'\n");
  if(!descriptor_indexing_features.runtimeDescriptorArray)
    printf("FATAL: driver missing 'runtime descriptor array'\n");
  
  VkDeviceCreateInfo dev_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pQueueCreateInfos = &q_create_info,
    .queueCreateInfoCount = 1,
    //.pEnabledFeatures = &dev_features,
    .enabledExtensionCount = DEV_EXT_C,
    .ppEnabledExtensionNames = DEV_EXT_NAMES,
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

int gfxRenderpassInit(GfxContext* gfx){
 VkAttachmentDescription color_attachment = {
    .format = cfg_format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  VkAttachmentReference color_attachment_ref = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };
 
  VkSubpassDescription subpass = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_attachment_ref,
  };
  
  VkSubpassDependency dependency = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
    
    .srcAccessMask = 0,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
  };
  
  VkRenderPassCreateInfo render_pass_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &color_attachment,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 1,
    .pDependencies = &dependency
  };

  if(vkCreateRenderPass(gfx->ldev, &render_pass_info, NULL, &gfx->renderpass) != VK_SUCCESS){
    printf("failed to creat renderpass");
    return 1;
  }
  return 0;
}

int gfxFramebufferInit(GfxContext* gfx){
   gfx->framebuffer = malloc(gfx->swapchain_c * sizeof(VkFramebuffer));

  for(size_t i = 0; i < gfx->swapchain_c; i++){

    VkFramebufferCreateInfo framebuffer_info = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = gfx->renderpass,
      .attachmentCount = 1,
      .pAttachments = &gfx->swapchain_views[i],
      .width = gfx->extent.width,
      .height = gfx->extent.height,
      .layers = 1,
    };

    VK_CHECK
      (vkCreateFramebuffer(gfx->ldev, &framebuffer_info,
			   NULL, &gfx->framebuffer[i] ));
  }
    
  gfx->frame_c = DRAW_BUFFER_COUNT;
  
  return 0;
}

int gfxRecreateSwapchain(void){

  GfxContext* gfx = gfxSetContext();
  
  vkDeviceWaitIdle(gfx->ldev);
  
  _gfxSwapchainDestroy(*gfx);
  
  gfxSwapchainInit(gfx);
  gfxFramebufferInit(gfx);

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
  
  gfx->render_bit = (VkSemaphore*)malloc( gfx->frame_c * sizeof(VkSemaphore));
  gfx->present_bit = (VkSemaphore*)malloc( gfx->frame_c * sizeof(VkSemaphore));
  gfx->fence = (VkFence*)malloc( gfx->frame_c * sizeof(VkFence));
  
  for(unsigned int i = 0; i < gfx->frame_c; i++){
  
    if(vkCreateSemaphore(gfx->ldev, &semaphore_info,
			 NULL, &gfx->render_bit[i]) != VK_SUCCESS ||
       vkCreateSemaphore(gfx->ldev, &semaphore_info,
			 NULL, &gfx->present_bit[i]) != VK_SUCCESS ||
       vkCreateFence(gfx->ldev, &fence_info,
		     NULL, &gfx->fence[i]) != VK_SUCCESS ){
      printf("!failed to create sync objects!\n");
      return 1;
    }
  }
  return 0;
}

int gfxCmdBuffersInit(GfxContext* gfx){
    gfx->cmd_buffer =
    (VkCommandBuffer*)malloc( gfx->frame_c * sizeof(VkCommandBuffer));
  
  for(unsigned int i = 0; i < gfx->frame_c; i++){
    gfx->cmd_buffer[i] = VK_NULL_HANDLE;
  }
  
  VkCommandBufferAllocateInfo allocate_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = gfx->cmd_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = gfx->frame_c,
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

int gfxPipelineInit(GfxContext* gfx){
    
  VkPipelineLayoutCreateInfo pipeline_layout_info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &gfx->texture_descriptors_layout,
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
  int attribute_count = 3;
  VkVertexInputAttributeDescription attribute_descriptions[attribute_count];

  // Position
  attribute_descriptions[0].binding = 0;
  attribute_descriptions[0].location = 0;
  attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
  attribute_descriptions[0].offset = offsetof(vertex2, pos);

  /* UV
   * normalised between 0-1, the floor of y also
   * describes texture index to sample from
   */
  attribute_descriptions[1].binding = 0;
  attribute_descriptions[1].location = 1;
  attribute_descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
  attribute_descriptions[1].offset = offsetof(vertex2, uv);

  // Colors, two 16 bit numbers packed into a 32bit
  attribute_descriptions[2].binding = 0;
  attribute_descriptions[2].location = 2;
  attribute_descriptions[2].format = VK_FORMAT_R32_UINT;
  attribute_descriptions[2].offset = offsetof(vertex2, fgIndex_bgIndex);

  VkVertexInputBindingDescription binding_description = {
    .binding = 0,
    .stride = sizeof(vertex2),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
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
    .renderPass = gfx->renderpass,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = -1,
  };

  if(vkCreateGraphicsPipelines
     (gfx->ldev, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &gfx->pipeline) != VK_SUCCESS) {
    printf("!failed to create graphics pipeline!\n");
  }
    
  vkDestroyShaderModule(gfx->ldev, frag_shader, NULL);
  vkDestroyShaderModule(gfx->ldev, vert_shader, NULL);
  
  return 0;
}
