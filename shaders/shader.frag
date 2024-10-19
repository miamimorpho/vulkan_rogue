#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0 ) uniform sampler2D texSampler[];

layout(location = 0) flat in uint textureIndex;
layout(location = 1) in vec2 textureUV;
layout(location = 2) flat in vec2 fgUV;
layout(location = 3) flat in vec2 bgUV;

layout(location = 0) out vec4 outColor;

void main() {

     vec4 fg = texture(texSampler[0], fgUV);
     vec4 bg = texture(texSampler[0], bgUV);

     vec2 normalUV = textureUV / 
     (textureSize(texSampler[textureIndex], 0) / vec2(8.0) );
     vec4 brightness = texture(texSampler[textureIndex], normalUV);
     //outColor = vec4(fgUV.x, bgUV.x, 0, 1);
     //outColor = texture(texSampler[textureIndex], normalUV);
     outColor = vec4(mix(bg, fg, brightness.r)); 
}
