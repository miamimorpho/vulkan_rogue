#ifndef VULKAN_META_H
#define VULKAN_META_H

#define VMA_DEBUG_LOG
#include "../extern/vk_mem_alloc.h"
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "maths.h"
#include "config.h"
#include <stdint.h>

typedef struct {
  VkBuffer handle;
  VmaAllocation allocation;
  VkDeviceSize used_size;
} GfxBuffer;

typedef struct {
  VkImage handle;
  VmaAllocation allocation;
  VkImageView view;
  VkSampler sampler;
} GfxImage;

typedef struct{
  GfxImage image;
  uint32_t* encoding;
  uint32_t glyph_c;
  uint32_t glyph_width;
  uint32_t glyph_height;
  uint32_t height;
  uint32_t width;
  uint32_t channels;
} GfxTileset;

typedef struct{
  /* const */ GLFWwindow* window;
  /* const */ VkInstance instance;
  /* const */ VkPhysicalDevice pdev;
  /* const */ VkDevice ldev;
  /* const */ VkQueue queue;
  /* const */ VmaAllocator allocator;
  /* const */ VkCommandPool cmd_pool;
  /* const */ VkSurfaceKHR  surface;
  /* const */ VkExtent2D extent;
  /* const */ VkSwapchainKHR swapchain;
  /* const */ uint32_t swapchain_c;
  VkImage *swapchain_images;
  VkImageView *swapchain_views;
  /* const */ uint32_t frame_c;
  /* const */ VkRenderPass renderpass;
  /* const */ VkFramebuffer *framebuffer;
  /* const */ VkDescriptorPool descriptor_pool;
  /* const */ VkCommandBuffer *cmd_buffer;
  /* const */ VkSemaphore* present_bit;
  /* const */ VkSemaphore* render_bit;
  /* const */ VkFence* fence;
  /* const */ VkDescriptorSetLayout texture_descriptors_layout;
  /* const */ VkDescriptorSet texture_descriptors;
  /* const */ VkPipelineLayout pipeline_layout;
  /* const */ VkPipeline pipeline;
}GfxContext;

typedef struct{
  vec2 screen_size_px;
}GfxPushConstant;

typedef struct{
  uint16_t glyph_code;
  uint16_t fg_index;
  uint16_t bg_index;
  uint16_t texture_index;
}GfxTile;

typedef struct{
    uint32_t swapchain_x;
    uint32_t frame_x;
    GfxTileset* textures;
    GfxBuffer vertices;

    GfxTile* tile_buffer;
    uint32_t tile_buffer_w;
    uint32_t tile_buffer_h;
}GfxGlobal;

typedef struct{
  vec2 pos;
  vec2 uv;
  uint32_t fgIndex_bgIndex;
} vertex2;

GfxContext* gfxSetContext(void);
GfxContext  gfxGetContext(void);

VkCommandBuffer gfxCmdSingleBegin(void);
int gfxCmdSingleEnd(VkCommandBuffer cmd_buffer);

uint32_t getUnicodeUV(GfxTileset, uint32_t);

/** Buffer and Image Functions */
int gfxVertBufferCreate(GfxContext, size_t, GfxBuffer*);
int gfxBufferAppend(VmaAllocator, GfxBuffer*,
		    const void*, VkDeviceSize);
GfxBuffer* gfxBufferNext(VmaAllocator, GfxBuffer*);
int gfxBufferCreate(VmaAllocator, VkBufferUsageFlags, VkDeviceSize, GfxBuffer*);
int gfxBufferDestroy(VmaAllocator, GfxBuffer*);

int gfxImageAlloc(VmaAllocator, GfxImage*, VkImageUsageFlags, VkFormat, uint32_t, uint32_t);
int gfxImageViewCreate(VkDevice, VkImage, VkImageView *, VkFormat, VkImageAspectFlags);
int copyBufferToImage(VkBuffer, VkImage, uint32_t, uint32_t);
int transitionImageLayout(VkImage, VkImageLayout, VkImageLayout);
void gfxImageDestroy(VmaAllocator, GfxImage);

/** Initialisation Functions */
int gfxRecreateSwapchain(void);
int gfxAllocatorInit(GfxContext*);
int gfxGlfwInit(GfxContext*);
int gfxInstanceInit(GfxContext*);
int gfxPhysicalDeviceInit(GfxContext*);
int gfxQueueIndex(GfxContext*);
int gfxLogicalDeviceInit(GfxContext*);
int gfxCmdPoolInit(GfxContext*);
int gfxSurfaceInit(GfxContext*);
int gfxSwapchainInit(GfxContext*);
int gfxDepthFormatCheck(GfxContext*);
int gfxRenderpassInit(GfxContext*);
int gfxFramebufferInit(GfxContext*);
int gfxDescriptorsPool(GfxContext*);
int gfxSyncInit(GfxContext*);
int gfxCmdBuffersInit(GfxContext*);
int gfxTextureDescriptorsInit(GfxContext*);
int gfxPipelineInit(GfxContext*);

int _gfxSwapchainDestroy(GfxContext);
int _gfxConstFree(GfxContext);

#endif // VULKAN_META_H
