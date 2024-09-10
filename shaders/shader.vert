#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in ivec3 inColor;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out ivec3 outColor;

void main() {

  outUV = inUV;
  outColor = inColor;
  gl_Position = vec4(inPosition, 0, 1.0);
  
}
