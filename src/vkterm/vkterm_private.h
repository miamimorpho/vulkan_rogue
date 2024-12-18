#ifndef VKTERM_PRIVATE_H
#define VKTERM_PRIVATE_H

#define VMA_DEBUG_LOG
#include "../../extern/vk_mem_alloc.h"
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vk_config.h"
#include <stdint.h>

typedef void (VKAPI_PTR *PFN_vkCmdBeginRenderingKHR)(VkCommandBuffer, const VkRenderingInfo*);
extern PFN_vkCmdBeginRenderingKHR pfn_vkCmdBeginRenderingKHR;
extern PFN_vkCmdEndRenderingKHR pfn_vkCmdEndRenderingKHR;

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

typedef uint32_t (*DecoderFunc)(uint32_t* encoding, uint32_t count, uint32_t unicode);

typedef struct{
  GfxImage image;
  uint32_t image_h;
  uint32_t image_w;
  uint32_t channels;
  
  uint32_t* encodings;
  DecoderFunc decoder;
  uint32_t glyph_c;
  uint32_t glyph_h;
  uint32_t glyph_w;
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
  /* const */ VkDescriptorPool descriptor_pool;
  /* const */ VkCommandBuffer *cmd_buffer;
  /* const */ VkSemaphore* image_available;
  /* const */ VkSemaphore* render_finished;
  /* const */ VkFence* fence;
  /* const */ VkDescriptorSetLayout texture_descriptors_layout;
  /* const */ VkDescriptorSet texture_descriptors;
  /* const */ VkPipelineLayout pipeline_layout;
  /* const */ VkPipeline pipeline;
  
}GfxContext;

typedef struct{
  uint32_t pos;
  uint32_t unicode_atlas_and_colors;
} GfxGlyph;

enum GfxLayerType{
    LAYER_TYPE_FREE,
    LAYER_TYPE_USED,
};

typedef struct{
  enum GfxLayerType type;    
  char* name;
  GfxGlyph* data;
  uint16_t count;
} GfxLayer;

typedef struct{
  GfxContext vk;
  uint32_t width_in_tiles;
  uint32_t height_in_tiles;
  uint32_t swapchain_x;
  uint32_t frame_x;
  
  GfxTileset* tilesets;
  
  GfxLayer* layers;
  int layer_x;
  int layer_c;
  GfxBuffer indirect;
  GfxBuffer gpu_glyph_cache;
}GfxGlobal;

/* Vulkan Extension Function */
VkResult init_vkCmdBeginRenderingKHR(VkDevice);

/* Command Buffer Singleshots */
VkCommandBuffer gfxCmdSingleBegin(GfxContext);
int gfxCmdSingleEnd(GfxContext, VkCommandBuffer);

/* Buffer and Image Functions */
int gfxVertBufferCreate(GfxContext, size_t, GfxBuffer*);
int gfxBufferAppend(VmaAllocator, GfxBuffer*,
		    const void*, VkDeviceSize);
size_t gfxBufferCapacity(VmaAllocator, GfxBuffer);
GfxBuffer* gfxBufferNext(VmaAllocator, GfxBuffer*);
int gfxBufferCreate(VmaAllocator, VkBufferUsageFlags, VkDeviceSize, GfxBuffer*);
int gfxBufferDestroy(VmaAllocator, GfxBuffer*);

int gfxImageAlloc(VmaAllocator, GfxImage*, VkImageUsageFlags, VkFormat, uint32_t, uint32_t);
int gfxImageViewCreate(VkDevice, VkImage, VkImageView *, VkFormat, VkImageAspectFlags);
int copyBufferToImage(GfxContext, VkBuffer, VkImage, uint32_t, uint32_t);
int transitionImageLayout(VkCommandBuffer, VkImage, VkImageLayout, VkImageLayout);
void gfxImageDestroy(VmaAllocator, GfxImage);

/* Initialise Auxillaries */
int gfxRecreateSwapchain(GfxContext* gfx);
int gfxPipelineInit(GfxContext*);
int gfxTilesetsMemoryInit(GfxTileset**);
void gfxInputInit(GLFWwindow* window);
int gfxLayerChange(GfxGlobal* gfx, const char* name);

/* Free Functions */
void layerBufferDestroy(GfxGlobal*);
int gfxTilesetsFree(GfxGlobal*);
int _gfxSwapchainDestroy(GfxContext);
int _gfxConstFree(GfxContext);

#endif // VULKAN_META_H
