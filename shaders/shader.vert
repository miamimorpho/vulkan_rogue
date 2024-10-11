#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in uint inFg;
layout(location = 3) in uint inBg;

layout(location = 0) out vec2 outUV;
layout(location = 1) flat out vec4 outFg;
layout(location = 2) flat out vec4 outBg;

vec4 unpackColor(uint hexColor) {
     vec4 dst;
    // Extract the RGBA components from the packed uint
    dst.a = float((hexColor >> 24) & 0xFF) / 255.0; // Alpha
    dst.r = float((hexColor >> 16) & 0xFF) / 255.0; // Red
    dst.g = float((hexColor >> 8) & 0xFF) / 255.0;  // Green
    dst.b = float(hexColor & 0xFF) / 255.0;         // Blue

    return dst;
}

void main() {

  outUV = inUV;

  outFg = unpackColor(inFg);
  outBg = unpackColor(inBg);

  gl_Position = vec4(inPosition, 0, 1.0);
  
}
