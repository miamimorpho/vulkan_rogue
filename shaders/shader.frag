#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0 ) uniform sampler2D texSampler[];

layout(location = 0) flat in uint texIndex;
layout(location = 1) in vec2 texUV;
layout(location = 2) flat in vec2 fgUV;
layout(location = 3) flat in vec2 bgUV;

layout(location = 0) out vec4 outColor;

void main() {

  // Color
  vec4 fg = texture(texSampler[0], fgUV);
  vec4 bg = texture(texSampler[0], bgUV);

  vec2 tile_size = vec2(8.0f) / textureSize(texSampler[texIndex], 0);
  vec2 normalUV = texUV * tile_size; 

  // Tileset Atlas Index
  vec4 brightness = texture(texSampler[texIndex], normalUV);

       //outColor = vec4(fgUV.x, bgUV.x, 0, 1);
       //outColor = texture(texSampler[textureIndex], normalUV);
  outColor = vec4(mix(bg, fg, brightness.r)); 
}
