#version 450

#define PALETTE_SIZE 16.0
#define TILE_SIZE 8

layout(location = 0) in uint inPosition;
layout(location = 1) in uint texEncoding;
layout(location = 2) in uint texIndexAndSize;
layout(location = 3) in uint fgIndex_bgIndex;

layout(location = 0) flat out uint texIndex;
layout(location = 1) out vec2 texUV;
layout(location = 2) flat out vec2 fgUV;
layout(location = 3) flat out vec2 bgUV;

layout( push_constant ) uniform PushConstants {
  vec2 screen_size;
} constants;

out gl_PerVertex {
  vec4 gl_Position;
};

/* Counter clockwise direction */
vec2 quadVertices[6] = {
  vec2(0, 0),
  vec2(0, 1),
  vec2(1, 0),
  vec2(0, 1),
  vec2(1, 1),
  vec2(1, 0)
};

vec2 encodingToUV(vec2 quadUV, uint encoding, uint width_in_tiles){
  uint tex_col = uint(mod(float(encoding), width_in_tiles));
  uint tex_row = encoding / width_in_tiles;
  vec2 uv_offset = vec2(tex_col, tex_row);

  return (quadUV + uv_offset);
}


ivec2 unpackUINT16(uint packed ){
      return ivec2(
      	     float((packed >> 16) & 0xFFFFu),
    	     float( packed        & 0xFFFFu)
	     );
}

void main() {

  // Position
  vec2 asciiPos =
  //unpackHalf2x16(inPosition) + quadVertices[gl_VertexIndex];
  unpackUINT16(inPosition) + quadVertices[gl_VertexIndex]; 
  vec2 asciiToScreen = vec2(TILE_SIZE) / constants.screen_size;
  vec2 ndcPos = 2 * asciiPos * asciiToScreen -1;
  gl_Position = vec4(ndcPos, 0, 1.0);

  // Texturing
  ivec2 texIndexAndSize_v = unpackUINT16(texIndexAndSize);
  texIndex = texIndexAndSize_v.x;
  int texSize = texIndexAndSize_v.y;
  
  texUV =
  encodingToUV(quadVertices[gl_VertexIndex], texEncoding, texSize);

  // Colour
  vec2 colorIndices = unpackUINT16(fgIndex_bgIndex);
  fgUV = vec2(float(colorIndices.x) / PALETTE_SIZE, 0.5);
  bgUV = vec2(float(colorIndices.y) / PALETTE_SIZE, 0.5);
}
