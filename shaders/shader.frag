#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0 ) uniform sampler2D texSampler[];

layout(location = 0) in vec2 inUV;
layout(location = 1) flat in ivec3 inColor;

layout(location = 1) out vec2 FragUV;
layout(location = 0) out vec4 outColor;

void main() {

    int texture_index = int(floor(inUV.y));
    vec2 textureUV = fract(inUV);

    vec4 brightness = texture(texSampler[texture_index], textureUV);
    vec3 normalColor = vec3(inColor) / 255;
    outColor = vec4(brightness.r * normalColor, 1.0);
    //outColor = vec4(brightness.r, brightness.r, brightness.r, 255);
}
