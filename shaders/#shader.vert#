#version 450

#define PALETTE_SIZE 16.0f

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in uint fgIndex_bgIndex;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out vec2 fgUV;
layout(location = 2) flat out vec2 bgUV;

layout( push_constant ) uniform constants {
  vec2 screen_size;
} PushConstants;

void main() {

  // cut the uint32 into uint16
  uint fgIndex = (fgIndex_bgIndex >> 16) & 0xFFFFu;
  uint bgIndex =  fgIndex_bgIndex        & 0xFFFFu;

  fgUV = vec2(float(fgIndex) / PALETTE_SIZE, 0.5);
  bgUV = vec2(float(bgIndex) / PALETTE_SIZE, 0.5);

  outUV = inUV;
  gl_Position = vec4(inPosition, 0, 1.0);
  
}
