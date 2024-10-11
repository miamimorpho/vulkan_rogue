#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0 ) uniform sampler2D texSampler[];

layout(location = 0) in vec2 inUV;
layout(location = 1) flat in vec4 fg;
layout(location = 2) flat in vec4 bg;

layout(location = 0) out vec4 outColor;

void main() {

     //vec4 color = texture(texSampler[0]

     // get texture index from uv y component
     int texture_index = int(floor(inUV.x));
     vec2 textureUV = fract(inUV);
     vec4 brightness = texture(texSampler[texture_index], textureUV);

     outColor = vec4(mix(bg, fg, brightness.r));
 
}
