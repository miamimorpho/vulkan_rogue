#version 450

#define ATLAS_WIDTH 32
#define PALETTE_SIZE 16.0
#define TILE_SIZE 8

layout(location = 0) in uint inPosition;
layout(location = 1) in uint unicode_atlas_and_colors;

layout(location = 0) out vec2 unicodeUV;
layout(location = 1) flat out uint atlas_index;
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

vec2 encodingToUV(vec2 quadUV, uint encoding){
  uint tex_col = uint(mod(float(encoding), ATLAS_WIDTH));
  uint tex_row = encoding / ATLAS_WIDTH;
  vec2 uv_offset = vec2(tex_col, tex_row);

  return (quadUV + uv_offset);
}

ivec2 unpackUINT16(uint packed ){
      return ivec2(
      	     float((packed >> 16) & 0xFFFFu),
    	     float( packed        & 0xFFFFu)
	     );
}

void main() {

  // Position
  vec2 asciiPos =
  unpackUINT16(inPosition) + quadVertices[gl_VertexIndex]; 
  vec2 asciiToScreen = vec2(TILE_SIZE) / constants.screen_size;
  vec2 ndcPos = 2 * asciiPos * asciiToScreen -1;
  gl_Position = vec4(ndcPos, 0, 1.0);

  // Glyph Selection
  uint unicode = (unicode_atlas_and_colors >> 22) & 0x3FFu;
  unicodeUV = encodingToUV(quadVertices[gl_VertexIndex], unicode);
  atlas_index = (unicode_atlas_and_colors >> 16) & 0x3Fu;

  // Color
  uint fg_index = (unicode_atlas_and_colors >> 8) & 0xFFu;
  uint bg_index =  unicode_atlas_and_colors & 0xFFu;
  fgUV = vec2(float(fg_index) / PALETTE_SIZE, 0.5);
  bgUV = vec2(float(bg_index) / PALETTE_SIZE, 0.5);

}
