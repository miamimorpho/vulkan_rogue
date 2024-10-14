#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in uint fgIndex_bgIndex;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out vec2 fgUV;
layout(location = 2) flat out vec2 bgUV;

void main() {

  uint fgIndex = (fgIndex_bgIndex >> 16) & 0xFFFFu;
  uint bgIndex =  fgIndex_bgIndex        & 0xFFFFu;

  fgUV = vec2(float(fgIndex) / 16.0f, 0.5);
  bgUV = vec2(float(bgIndex) / 16.0f, 0.5);

  outUV = inUV;
  gl_Position = vec4(inPosition, 0, 1.0);
  
}
