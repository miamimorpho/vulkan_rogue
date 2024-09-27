#define STB_IMAGE_IMPLEMENTATION
#include "../extern/stb_image.h"
#include "textures.h"
#include "vulkan_helper.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef int (*Opener_f)(const char* filename, GfxTileset* img, uint8_t** pixels, size_t* size);

int
gfxTexturesDescriptorsUpdate(GfxConst global, GfxTileset* textures, uint32_t count)
{
  VkDescriptorImageInfo infos[count];
  VkWriteDescriptorSet writes[count];

  for(uint32_t i = 0; i < count; i++){

    GfxImage* texture = &textures[i].image;
    if(texture->handle == VK_NULL_HANDLE)printf("texture loading error\n");

    infos[i] = (VkDescriptorImageInfo){
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      .imageView = texture->view,
      .sampler = texture->sampler,
    };
    
    writes[i] = (VkWriteDescriptorSet){
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = global.texture_descriptors,
      .dstBinding = 0,
      .dstArrayElement = i,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = 1,
      .pImageInfo = &infos[i],
    };
  }

  vkUpdateDescriptorSets(global.ldev, count, writes, 0, NULL); 
  
  return 0;
}

int
gfxSamplerCreate(VkDevice ldev, VkPhysicalDevice pdev, VkSampler *sampler)
{

  VkPhysicalDeviceProperties properties = { 0 };
  vkGetPhysicalDeviceProperties(pdev, &properties);
  
  VkSamplerCreateInfo sampler_info = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = VK_FILTER_NEAREST,
    .minFilter = VK_FILTER_NEAREST,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .anisotropyEnable = VK_FALSE,
    .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE,
    .compareEnable = VK_FALSE,
    .compareOp = VK_COMPARE_OP_ALWAYS,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    .mipLodBias = 0.0f,
    .minLod = 0.0f,
    .maxLod = 0.0f,
  };

  if(vkCreateSampler(ldev, &sampler_info, NULL, sampler)
     != VK_SUCCESS){
    printf("!failed to create buffer\n!");
    return 1;
  }
  
  return 0;
  
}

int
copyBufferToImage(VkBuffer buffer, VkImage image,
		  uint32_t width, uint32_t height)
{
  VkCommandBuffer command = gfxCmdSingleBegin();
  
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
  
  return gfxCmdSingleEnd(command);
}

int
transitionImageLayout(VkImage image,
			VkImageLayout old_layout, VkImageLayout new_layout){
  VkCommandBuffer commands = gfxCmdSingleBegin();

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
  
  if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED
     && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  }else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	    && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    
  } else {
    printf("unsupported layout transition\n");
    return 1;
  }
  
  vkCmdPipelineBarrier(commands,
		       source_stage, destination_stage,
		       0,
		       0, NULL,
		       0, NULL,
		       1, &barrier);
  
  return gfxCmdSingleEnd(commands);
}

int
gfxImageToGpu(VmaAllocator allocator, unsigned char* pixels,
	     int width, int height, int channels,
	     GfxImage *texture){

  VmaAllocatorInfo allocator_info;
  vmaGetAllocatorInfo(allocator, &allocator_info);

  VkFormat format;
  switch(channels){
  case 4:
    format = VK_FORMAT_R8G8B8A8_SRGB;
    break;
  case 3:
    format = VK_FORMAT_R8G8B8_SRGB;
    break;
  case 1:
    format = VK_FORMAT_R8_UNORM;
    break;
  default:
    return 2;
  }
  
  VkDeviceSize image_size = width * height * channels;
  GfxBuffer image_b;
  gfxBufferCreate(allocator,
		  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		  image_size, &image_b);
  
  
  /* copy pixel data to buffer */
  vmaCopyMemoryToAllocation(allocator,
			    pixels,
			    image_b.allocation, 0,
			    image_size);
  //memcpy(image_b.first_ptr, pixels, image_size);

  gfxImageAlloc(allocator, texture, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, format, width, height);

  /* Copy the image buffer to a VkImage proper */
  transitionImageLayout(texture->handle,
			  VK_IMAGE_LAYOUT_UNDEFINED,
			  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);


  copyBufferToImage(image_b.handle, texture->handle,
		    (uint32_t)width, (uint32_t)height);
  
  gfxBufferDestroy(allocator, &image_b);

  transitionImageLayout
    (texture->handle,
     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  
  /* Create image view */
  gfxImageViewCreate(allocator_info.device, texture->handle,
		    &texture->view, format,
		    VK_IMAGE_ASPECT_COLOR_BIT);
  gfxSamplerCreate(allocator_info.device,
		 allocator_info.physicalDevice,
		 &texture->sampler);
 
  return 0;
}

int pngFileLoad(const char* filename, GfxTileset* img, uint8_t** pixels, size_t* size){

  int x, y, n;
  stbi_info(filename, &x, &y, &n);
  *pixels = stbi_load(filename, &x, &y, &n, n);
  if(*pixels == NULL){
    return 1;
  }
  *size = x * y * n;
  img->width = (uint32_t)x;
  img->height = (uint32_t)y;
  img->glyph_width = TILE_SIZE;
  img->glyph_height = TILE_SIZE;
  img->channels = n;
  
  return 0;
}

int bdfFileLoad(const char* filename, GfxTileset* font, uint8_t** ptr_pixels, size_t* size ){

  const int L_LEN = 80;
  const int W_LEN = 8;
  
  FILE *fp = fopen(filename, "r");
  if(fp == NULL) {
    printf("Error opening font file\n");
    return 1;
  }
 
  char line[L_LEN];
  char setting[L_LEN];
  
  uint32_t glyph_c;
  int supported = 0;
  /* METADATA LOADING START */
  while(fgets(line, sizeof(line), fp) != NULL) {

    if(supported == 3)break;
    
    sscanf(line, " %s", setting);

    if(strcmp(setting, "FONTBOUNDINGBOX") == 0){

      char x_s[W_LEN];
      char y_s[W_LEN];
      char x_offset_s[W_LEN];
      char y_offset_s[W_LEN];
      
      sscanf(line, "%*s %s %s %s %s",
	     x_s, y_s, x_offset_s, y_offset_s);

      font->glyph_width = atoi(x_s);
      font->height = atoi(y_s);
  
      supported += 1;
      continue;
    }

    if(strcmp(setting, "CHARSET_ENCODING") == 0){
      char encoding[W_LEN]; 
      
      sscanf(line, "%*s %s", encoding);
      if(strcmp(encoding, "\"437\"") != 0 )printf("unsupported font format");

      supported += 1;
      continue;
    }

    if(strcmp(setting, "CHARS") == 0){
      char glyph_c_s[W_LEN];
      sscanf(line, "%*s %s", glyph_c_s);
      glyph_c = atoi(glyph_c_s);
      supported += 1;
      continue;
    }
   
  }
  
  if(supported != 3){
    printf("not enough config information\n");
    return 2;
  }/* META LOADING END */

  font->channels = 1;
  font->width = font->glyph_width * glyph_c;
  *size = font->width * font->height;
  *ptr_pixels = malloc(*size);
  font->glyph_height = font->height;
  memset(*ptr_pixels, 255, *size);
  
  rewind(fp);
  int in_bytes = 0;
  int row_i = 0;
  int code_i = 0;

  /* BYTE LOADING START */
  while(fgets(line, sizeof(line), fp) != NULL) {
 
    sscanf(line, "%s", setting);

    if(strcmp(setting, "ENCODING") == 0){
      char code[W_LEN];
      sscanf(line, "%*s %s", code);
      code_i = atoi(code);
 
   
      continue;
    }
    
    if(strcmp(setting, "ENDCHAR") == 0){
      row_i = 0;
      code_i = 0;
      in_bytes = 0;
    
      continue;
    }
    
    if(in_bytes == 1){
   
      uint8_t row = (uint8_t)strtol(line, NULL, 16);
      int y = row_i++ * (font->width);
    
      for(signed int i = 7; i >= 0; --i){
	int pixel_xy = y + (code_i * font->glyph_width) + i;
	uint8_t pixel = 0;
	if((row >> (7 - i) ) & 0x01)pixel = 255;	
	(*ptr_pixels)[pixel_xy] = pixel;
      }
   
      continue;
    }
      
    if(strcmp(setting, "STARTCHAR") == 0){
      char* glyph_name = strchr(line, ' ');
      if(glyph_name != NULL)glyph_name += 1;
      glyph_name[strlen(glyph_name)-1] = '\0';
 
      continue;
    }

    if(strcmp(setting, "BITMAP") == 0){
      in_bytes = 1;
   
      continue;
    }
  }/* BYTE LOADING END*/

  fclose(fp);
  return 0;  
}

int errFileLoad(const char* filename, GfxTileset* img, uint8_t** ptr_pixels, size_t* size){
  printf("error loading texture %s\n", filename);
  return 1;
}

Opener_f getFileOpener(const char *filename) {
    char *ext = strrchr(filename, '.');
    if(!ext || ext == filename) return &errFileLoad;
    ext++;
    
    if(strcmp(ext, "png") == 0){
      return &pngFileLoad;
    }
    if(strcmp(ext, "bdf") == 0){
      return &bdfFileLoad;
    }
    return &errFileLoad;
}

int _gfxTextureLoad(GfxConst gfx, const char* filename, GfxTileset* textures){

  if(textures == NULL){
    return 1;
  }
  
  // search for empty font texture slot
  int index = -1;
  for(uint32_t i = 0; i < MAX_SAMPLERS; i++){

    if(textures[i].image.handle == VK_NULL_HANDLE){
      index = i;
      break;
    }
  }
  GfxTileset* texture = &textures[index];

  /* load pixels from file */
  uint8_t* pixels = NULL;
  size_t size = 0;
  Opener_f imageOpen = getFileOpener(filename);
  int err = imageOpen(filename, texture, &pixels, &size);
  if(err > 0){
    printf("error opening file, err = %d\n", err);
    return -1;
  }

  /* send image to GPU */
  if(gfxImageToGpu(gfx.allocator,
		   pixels,
		   texture->width, texture->height,
		   texture->channels, &texture->image) < 0) return 1;
  gfxTexturesDescriptorsUpdate(gfx, textures, index +1);
  
  /* stbi_free() is just a wrapper for free() */
  free(pixels);
  
  return 0;
}

int _gfxTexturesInit(GfxTileset** textures){
  *textures = (GfxTileset*)malloc(MAX_SAMPLERS * sizeof(GfxTileset));
  for(unsigned int i = 0; i < MAX_SAMPLERS; i++){
    (*textures)[i].glyph_height = 0;
    (*textures)[i].glyph_width = 0;
    (*textures)[i].width = 0;
    (*textures)[i].height = 0;
    (*textures)[i].image.handle = VK_NULL_HANDLE;
    (*textures)[i].image.view = VK_NULL_HANDLE;
  }
  return 0;
}
