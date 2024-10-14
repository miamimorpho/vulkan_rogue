#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0 ) uniform sampler2D texSampler[];

layout(location = 0) in vec2 textureIndexAndUV;
layout(location = 1) flat in vec2 fgUV;
layout(location = 2) flat in vec2 bgUV;

layout(location = 0) out vec4 outColor;

void main() {

     vec4 fg = texture(texSampler[0], fgUV);
     vec4 bg = texture(texSampler[0], bgUV);

     int texture_index = int(floor(textureIndexAndUV.x));    
     vec2 textureUV = fract(textureIndexAndUV);

     vec4 brightness = texture(texSampler[texture_index], textureUV);
     outColor = vec4(mix(bg, fg, brightness.r));
 
}
