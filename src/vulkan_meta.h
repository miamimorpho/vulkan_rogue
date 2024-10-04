#ifndef VULKAN_META_H
#define VULKAN_META_H

#define VMA_DEBUG_LOG
#include "../extern/vk_mem_alloc.h"
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "maths.h"
#include "config.h"

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
  /* const */ VkSurfaceKHR surface;
  /* const */ VkExtent2D extent;
  /* const */ VkSwapchainKHR swapchain;
  /* const */ uint32_t swapchain_c;
  /* const */ VkImage *swapchain_images;
  /* const */ VkImageView *swapchain_views;
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
}GfxConst;

typedef struct{
    uint32_t swapchain_x;
    uint32_t frame_x;
    GfxTileset* textures;
    GfxBuffer vertices;
}GfxGlobal;

typedef struct{
  vec2 pos;
  vec2 uv;
  ivec3 color;
} vertex2;

GfxConst* gfxSetConst(void);
GfxConst  gfxGetConst(void);

VkCommandBuffer gfxCmdSingleBegin(void);
int gfxCmdSingleEnd(VkCommandBuffer cmd_buffer);
int _gfxConstFree(GfxConst);

#endif // VULKAN_META_H
