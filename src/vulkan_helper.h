#include "vulkan_meta.h"
#include "macros.h"

uint32_t getUnicodeUV(GfxTileset, uint32_t);
int gfxVertBufferCreate(GfxContext, size_t, GfxBuffer*);
int gfxBufferAppend(VmaAllocator, GfxBuffer*,
		    const void*, VkDeviceSize);
GfxBuffer* gfxBufferNext(VmaAllocator, GfxBuffer*);
int gfxBufferCreate(VmaAllocator, VkBufferUsageFlags, VkDeviceSize, GfxBuffer*);
int gfxBufferDestroy(VmaAllocator, GfxBuffer*);

int gfxImageAlloc(VmaAllocator, GfxImage*, VkImageUsageFlags, VkFormat, uint32_t, uint32_t);
int gfxImageViewCreate(VkDevice, VkImage, VkImageView *, VkFormat, VkImageAspectFlags);
void gfxImageDestroy(VmaAllocator allocator, GfxImage image);

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
