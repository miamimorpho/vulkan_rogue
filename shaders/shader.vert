#version 450

#define PALETTE_SIZE 16.0
#define TILE_SIZE 8

layout(location = 0) in uint inPosition;
layout(location = 1) in uint textureEncoding;
layout(location = 2) in uint textureIndexIn;
layout(location = 3) in uint fgIndex_bgIndex;

layout(location = 0) flat out uint textureIndexOut;
layout(location = 1) out vec2 textureUV;
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

vec2 unpackUINT16(uint packed ){
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
  textureIndexOut = textureIndexIn; 
  textureUV =
  (unpackUINT16(textureEncoding) + quadVertices[gl_VertexIndex]);

  // Colour
  vec2 colorIndices = unpackUINT16(fgIndex_bgIndex);
  fgUV = vec2(float(colorIndices.x) / PALETTE_SIZE, 0.5);
  bgUV = vec2(float(colorIndices.y) / PALETTE_SIZE, 0.5);
}
