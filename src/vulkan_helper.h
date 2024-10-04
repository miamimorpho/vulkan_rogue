#include "vulkan_meta.h"

int getUnicodeUV(GfxTileset, unsigned int);
int gfxVertBufferCreate(GfxConst, size_t, GfxBuffer*);
int gfxBufferAppend(VmaAllocator, GfxBuffer*,
		    const void*, VkDeviceSize);
GfxBuffer* gfxBufferNext(VmaAllocator, GfxBuffer*);
int gfxBufferCreate(VmaAllocator, VkBufferUsageFlags, VkDeviceSize, GfxBuffer*);
int gfxBufferDestroy(VmaAllocator, GfxBuffer*);

int gfxImageAlloc(VmaAllocator, GfxImage*, VkImageUsageFlags, VkFormat, uint32_t, uint32_t);
int gfxImageViewCreate(VkDevice, VkImage, VkImageView *, VkFormat, VkImageAspectFlags);
void gfxImageDestroy(VmaAllocator allocator, GfxImage image);

int gfxAllocatorInit(GfxConst*);
int gfxGlfwInit(GfxConst*);
int gfxInstanceInit(GfxConst*);
int gfxPhysicalDeviceInit(GfxConst*);
int gfxQueueIndex(GfxConst*);
int gfxLogicalDeviceInit(GfxConst*);
int gfxCmdPoolInit(GfxConst*);
int gfxSurfaceInit(GfxConst*);
int gfxSwapchainInit(GfxConst*);
int gfxDepthFormatCheck(GfxConst*);
int gfxRenderpassInit(GfxConst*);
int gfxFramebufferInit(GfxConst*);
int gfxDescriptorsPool(GfxConst*);
int gfxSyncInit(GfxConst*);
int gfxCmdBuffersInit(GfxConst*);
int gfxTextureDescriptorsInit(GfxConst*);
int gfxPipelineInit(GfxConst*);
