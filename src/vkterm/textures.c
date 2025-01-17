#define STB_IMAGE_IMPLEMENTATION
#include "../../extern/stb_image.h"
#include "vkterm_private.h"
#include "macros.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef int (*OpenerFunc)(const char* filename, GfxTileset* img, uint8_t** pixels, size_t* size);

int bsearchCompare(const void *a, const void *b) {
    return (*(uint32_t *)a - *(uint32_t *)b);
}

uint32_t DecoderBinarySearch(uint32_t* encodings, uint32_t count, uint32_t unicode){
  uint32_t* result = (uint32_t *)bsearch(&unicode, encodings, count, sizeof(uint32_t), bsearchCompare);
  if(result != NULL)
    return result - encodings;

  return 0;
}

uint32_t DecoderLinearSearch(uint32_t* encodings, uint32_t count, uint32_t unicode){
  if(unicode > count) return 0;
  for(uint32_t i = 0; i < count; i++){
    if(encodings[i] == unicode){
      return i;
    }
  }
  return 0;
}

uint32_t DecoderArrayGet(uint32_t* encodings, uint32_t count, uint32_t unicode){
  if(unicode < count){
    return encodings[unicode];
  }
  return 0;
}

int
gfxTexturesDescriptorsUpdate(GfxContext global, GfxTileset* textures, uint32_t count)
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
gfxImageToGpu(GfxContext vk, unsigned char* pixels,
	     int width, int height, int channels,
	     GfxImage *texture){

  VmaAllocatorInfo allocator_info;
  vmaGetAllocatorInfo(vk.allocator, &allocator_info);

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
  gfxBufferCreate(vk.allocator,
		  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		  image_size, &image_b);
  
  
  /* copy pixel data to buffer */
  vmaCopyMemoryToAllocation(vk.allocator,
			    pixels,
			    image_b.allocation, 0,
			    image_size);
  //memcpy(image_b.first_ptr, pixels, image_size);

  CHECK_GT_ZERO
    (gfxImageAlloc(vk.allocator, texture,
		   VK_IMAGE_USAGE_TRANSFER_DST_BIT
		   | VK_IMAGE_USAGE_SAMPLED_BIT,
		   format, width, height));

  /* Copy the image buffer to a VkImage proper */
  VkCommandBuffer cmd = gfxCmdSingleBegin(vk);
  CHECK_GT_ZERO
    (transitionImageLayout(cmd, texture->handle,
			   VK_IMAGE_LAYOUT_UNDEFINED,
			   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
  gfxCmdSingleEnd(vk, cmd);

  CHECK_GT_ZERO
    (copyBufferToImage(vk, image_b.handle, texture->handle,
		       (uint32_t)width, (uint32_t)height));
  
  gfxBufferDestroy(vk.allocator, &image_b);

  cmd = gfxCmdSingleBegin(vk);
  CHECK_GT_ZERO(transitionImageLayout
		(cmd, texture->handle,
     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
  gfxCmdSingleEnd(vk, cmd);
  
  /* Create image view */
  CHECK_GT_ZERO
    (gfxImageViewCreate(allocator_info.device, texture->handle,
			&texture->view, format,
			VK_IMAGE_ASPECT_COLOR_BIT));
  CHECK_GT_ZERO
    (gfxSamplerCreate(allocator_info.device,
		      allocator_info.physicalDevice,
		      &texture->sampler));

  return 0;
}

int pngFileLoad(const char* filename, GfxTileset* tileset, uint8_t** pixels, size_t* size){

  int x, y, n;
  stbi_info(filename, &x, &y, &n);
  *pixels = stbi_load(filename, &x, &y, &n, n);
  if(*pixels == NULL){
    printf("%s\n", stbi_failure_reason());
    return 1;
  }
  *size = x * y * n;
  tileset->image_w = (uint32_t)x;
  tileset->image_h = (uint32_t)y;
  tileset->glyph_w = ASCII_TILE_SIZE;
  tileset->glyph_h = ASCII_TILE_SIZE;
  tileset->glyph_c = ( x * y ) / ( ASCII_TILE_SIZE * ASCII_TILE_SIZE) ;
  tileset->channels = n;
  
  tileset->encodings = malloc(tileset->glyph_c * sizeof(uint32_t));
  for(uint32_t i = 0; i < tileset->glyph_c; i++){
    tileset->encodings[i] = i;
  }
  tileset->decoder = &DecoderArrayGet;
  
  return 0;
}

int HexToUINT4(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

int bdfFileLoad(const char* filename, GfxTileset* font, uint8_t** ptr_pixels, size_t* size ){

  const int L_LEN = 80;
  const int W_LEN = 8;
  char x_s[W_LEN];
  char y_s[W_LEN];
  char x_offset_s[W_LEN];
  char y_offset_s[W_LEN];

  int font_y_offset;
  
  FILE *fp = fopen(filename, "r");
  if(fp == NULL) {
    printf("Error opening font file\n");
    return 1;
  }

  // bdf files are parsed line by plaintext line
  char line[L_LEN];
  char prefix[L_LEN];
  
  int supported_c = 0;
  /* METADATA LOADING START */
  while(fgets(line, sizeof(line), fp) != NULL) {

    if(supported_c == 2)break;
    
    sscanf(line, " %s", prefix);

    if(strcmp(prefix, "FONTBOUNDINGBOX") == 0){
      
      sscanf(line, "%*s %s %s %s %s",
	     x_s, y_s, x_offset_s, y_offset_s);

      font->glyph_w = atoi(x_s);
      font->glyph_h = atoi(y_s);
      font_y_offset = atoi(y_offset_s);
   
      supported_c += 1;
      continue;
    }

    if(strcmp(prefix, "CHARS") == 0){
      char glyph_c_s[W_LEN];
      sscanf(line, "%*s %s", glyph_c_s);
      font->glyph_c = atoi(glyph_c_s);
      supported_c += 1;
      continue;
    }
   
  }
  if(supported_c != 2){
    printf("not enough config information\n");
    return 2;
  }/* META LOADING END */

  font->encodings = malloc(font->glyph_c * sizeof(uint32_t));
  
  // allocate pixel data to Z order array
  font->channels = 1;
  font->image_w = font->glyph_w * ATLAS_WIDTH;
  font->image_h = font->glyph_h * ATLAS_WIDTH;
  *size = font->image_w * font->image_h;
  *ptr_pixels = malloc(*size);
  memset(*ptr_pixels, 0, *size);

  rewind(fp);
  int glyph_i = 0; // assuming C99 uses ASCII 437 
  int glyph_y = -1;

  int bbx_size = 0;
  int bby_size = 0;
  int bbx_offset = 0;
  int bby_offset = 0;
 
  /* BYTE LOADING START */
  while(fgets(line, sizeof(line), fp) != NULL) {
 
    sscanf(line, "%s", prefix);
    
    if(strcmp(prefix, "ENDCHAR") == 0){
      glyph_i++;
      glyph_y = -1;
      continue;
    }

    /* bdf files store 4 1-bit pixels across
     * as one hex char */
    if(glyph_y >= 0){

      //int width_in_tiles = font->glyph_w * ATLAS_WIDTH;
      int atlas_y = glyph_i / ATLAS_WIDTH;
      int atlas_x = glyph_i % ATLAS_WIDTH;
      
      int dst_y =  (atlas_y * font->glyph_h) + font_y_offset; // atlas pixel pos
          dst_y += (8 - bby_size  - bby_offset); // local pixel pos
      int dst_x =  (atlas_x * font->glyph_w);
          dst_x += (8 - bbx_size) - bbx_offset;
      
      for(unsigned int i = 0; i < font->glyph_w; i++) {

	// read in one binary pixel
	uint8_t four_pixels = HexToUINT4(line[ i / 4]);
	int pixel_index = 3 - (i % 4);
	uint8_t dst_pixel = ((four_pixels >> pixel_index) & 0x1);

	// draw in one uint8 pixel	
	int dst_xy = ((dst_y + glyph_y ) * font->image_w) + dst_x + i;
	(*ptr_pixels)[dst_xy] = dst_pixel * 255;
      }
      glyph_y++;
      continue;
    }
 
    if(strcmp(prefix, "STARTCHAR") == 0){
      char* glyph_name = strchr(line, ' ');
      if(glyph_name != NULL)glyph_name += 1;
      glyph_name[strlen(glyph_name)-1] = '\0';
      continue;
    }
    
    if(strcmp(prefix, "ENCODING") == 0){
      char code[W_LEN];
      sscanf(line, "%*s %s", code);
      font->encodings[glyph_i] = atoi(code);
      continue;
    }

    if(strcmp(prefix, "BBX") == 0){
      sscanf(line, "%*s %s %s %s %s", x_s, y_s, x_offset_s, y_offset_s);
      bbx_size = atoi(x_s);
      bby_size = atoi(y_s);
      bbx_offset = atoi(x_offset_s);
      bby_offset = atoi(y_offset_s);
      continue;
    }
    
    if(strcmp(prefix, "BITMAP") == 0){
      glyph_y = 0;
      continue;
    }
  }/* BYTE LOADING END*/
  font->decoder = &DecoderBinarySearch;
  
  fclose(fp);
  return 0;  
}

int errFileLoad(const char* filename, GfxTileset* img, uint8_t** ptr_pixels, size_t* size){
  printf("error loading texture %s\n", filename);
  return 1;
}

OpenerFunc getFileOpener(const char *filename) {
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

int gfxTextureLoad(GfxGlobal* gfx, const char* filename){

  GfxTileset* textures = gfx->tilesets;
  if(textures == NULL){
    return 1;
  }
  int texture_index = -1;
  while(textures[++texture_index].image.handle != NULL);
  if(texture_index == -1 || texture_index > MAX_SAMPLERS) return 1;
  GfxTileset* texture = &textures[texture_index];
  
  /* load pixels from file */
  uint8_t* pixels = NULL;
  size_t size = 0;
  OpenerFunc imageOpen = getFileOpener(filename);
  int err = imageOpen(filename, texture, &pixels, &size);
  if(err > 0){
    printf("error opening file, err = %d\n", err);
    return -1;
  }

  /* send image to GPU */
  if(gfxImageToGpu(gfx->vk,
		   pixels,
		   texture->image_w, texture->image_h,
		   texture->channels, &texture->image) < 0) return 1;
  gfxTexturesDescriptorsUpdate(gfx->vk, textures, texture_index +1);
  /* stbi_free() is just a wrapper for free() */
  free(pixels);
  
  return 0;
}

int gfxTilesetsMemoryInit(GfxTileset** tileset_arr){
  *tileset_arr = (GfxTileset*)malloc(MAX_SAMPLERS * sizeof(GfxTileset));
  for(unsigned int i = 0; i < MAX_SAMPLERS; i++){
    (*tileset_arr)[i].glyph_h = 0;
    (*tileset_arr)[i].glyph_w = 0;
    (*tileset_arr)[i].image_w = 0;
    (*tileset_arr)[i].image_h = 0;
    (*tileset_arr)[i].image.handle = VK_NULL_HANDLE;
    (*tileset_arr)[i].image.view = VK_NULL_HANDLE;
  }
  return 0;
}

int gfxTilesetsFree(GfxGlobal* gfx){
  for(int i = 0; i < MAX_SAMPLERS; i++){
      if(gfx->tilesets[i].image.handle != VK_NULL_HANDLE){
          gfxImageDestroy(gfx->vk.allocator, gfx->tilesets[i].image);
          free(gfx->tilesets[i].encodings);
      }
  }
  free(gfx->tilesets);
  return 0;
}
