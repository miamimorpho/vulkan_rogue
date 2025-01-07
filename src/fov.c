#include<stdlib.h>

#include "fov.h"
#include "mystdlib.h"
#include "maths.h"

enum {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};

typedef struct {
  int cardinal;
  int depth;
  Fraction start_slope;
  Fraction end_slope;
} Row;

struct ShadowcastFnCtx{
    Gfx gfx;
    int x_offset;
    int y_offset;
    Bitmap* dst_mask;
};

typedef void (*ShadowcastFnPtr)(void*, MapPosition);
struct ShadowcastInterface{
    ShadowcastFnPtr fn;
    void* ctx;
};

// TODO, static module based stack

void transformByCardinal(int cardinal, int ox, int oy, int row, int col, int* out_x, int* out_y) {
    switch(cardinal) {
        case NORTH:
            *out_x = ox + col;
            *out_y = oy - row;
            break;
        case SOUTH:
            *out_x = ox + col;
            *out_y = oy + row;
            break;
        case EAST:
            *out_x = ox + row;
            *out_y = oy + col;
            break;
        case WEST:
            *out_x = ox - row;
            *out_y = oy + col;
            break;
    }
}

int getCardinal(int center_x, int center_y, int test_x, int test_y) {
    int dx = test_x - center_x;
    int dy = test_y - center_y;
    
    // If horizontal distance is greater than vertical
    if (abs(dx) > abs(dy)) {
        return dx > 0 ? EAST : WEST;
    } 
    // If vertical distance is greater or equal
    else {
        return dy > 0 ? SOUTH : NORTH;
    }
}

// Helper functions for rounding
static int round_ties_up(double n) {
      return (int)floor(n + 0.5);
}

static int round_ties_down(double n) {
   return (int)ceil(n - 0.5);
}

static Fraction slope(int row_depth, int col) {
    return fractionNew(2 * col - 1, 2 * row_depth);
}

static bool isSymmetric(Row* row, int col)
{
  /* checks if slope (col/row) is 
   * between start and end slopes at all*/
  int depth_mul_start = row->depth * row->start_slope.num;
  int col_div_start = col * row->start_slope.den;
  
  int depth_mul_end = row->depth * row->end_slope.num;
  int col_div_end = col * row->end_slope.den;
  
  return (col_div_start >= depth_mul_start &&
	  col_div_end <= depth_mul_end);
}

void shadowcastMarkVisible(struct ShadowcastFnCtx* ctx, MapPosition pos) {
    // TODO replace with direct rendering
    int scr_x = pos.x - ctx->x_offset;
    int scr_y = pos.y - ctx->y_offset;
    struct GameObjectTile t = terraGetTile(pos);

    gfxRenderGlyph(ctx->gfx, scr_x, scr_y, 
                   t.unicode, t.atlas, t.fg, t.bg);
    bitmapSetPx(ctx->dst_mask, pos.x, pos.y, 1);
}

static void shadowcastScanRow(struct ShadowcastInterface effect, 
                              MapPosition camera, Row current_row){

    // TODO, replace all stack based allocations here, we cant detect OOM on stack allocations
  static float SHADOWCAST_MAX = 12.5 * 12.5;

  // allows early termination on invalid angles
  // kind of a hack?
  if (fractionCompare(current_row.end_slope, current_row.start_slope)) {
    return;
  }

  // depth * slope
  int min_col = round_ties_up( (double)
    (current_row.depth * current_row.start_slope.num)
    / (double)current_row.start_slope.den);
  int max_col = round_ties_down( (double)
    (current_row.depth * current_row.end_slope.num)
    / (double)current_row.end_slope.den);
  
  bool prev_was_wall = false;
 
  for(int col = min_col; col <= max_col; col++) {    
    int target_x, target_y;
    transformByCardinal(current_row.cardinal, camera.x, camera.y, 
		     current_row.depth, col, &target_x, &target_y);

    MapPosition current_pos = 
      { target_x, target_y, camera.chunk_ptr };
    uint8_t is_wall = terraDoesBlockSight(current_pos);
    
    if(is_wall || isSymmetric(&current_row, col)) {
      if(relativeDistance(current_row.depth, col) < SHADOWCAST_MAX)
          effect.fn(effect.ctx, current_pos);
    }
    
    if(prev_was_wall && !is_wall) {
      current_row.start_slope = slope(current_row.depth, col);
    }
    
    if(!prev_was_wall && is_wall) {
      Row next_row = {
          .cardinal = current_row.cardinal,
          .depth = current_row.depth + 1,
          .start_slope = current_row.start_slope,
          .end_slope = slope(current_row.depth, col)
      };
      shadowcastScanRow(effect, camera, next_row);
    }
    
    prev_was_wall = is_wall;
  }// end of row scanning

  if(!prev_was_wall) {
    Row next_row = {
        .cardinal = current_row.cardinal,
        .depth = current_row.depth + 1,
        .start_slope = current_row.start_slope,
        .end_slope = current_row.end_slope
    };
    shadowcastScanRow(effect, camera, next_row);
  }
  
}

void shadowcastFOV(struct ShadowcastInterface inter, MapPosition camera){
 
  inter.fn(inter.ctx, camera);
  for(int cardinal = 0; cardinal < 4; cardinal++) {
    
    Row first_row = {
      .cardinal = cardinal,
      .depth = 1,
      .start_slope = fractionNew(-1, 1),
      .end_slope = fractionNew(1, 1)
    };
    shadowcastScanRow(inter, camera, first_row);
    
  }
}

int mapChunkDraw(Gfx gfx, struct WorldArena* arena, MapPosition camera){

  gfxClear(gfx);
  int scr_width = gfxGetScreenWidth(gfx);
  int scr_height = gfxGetScreenHeight(gfx);

  int x_offset = camera.x - (scr_width / 2);
  int y_offset = camera.y - (scr_height / 2);

  AllocatorInterface allocator = arena->allocator;
  Bitmap* mask = bitmapCreate(scr_width, scr_height, allocator);

  /* First chunk rendering pass
   * draws terrain to the screen and a visibility mask
   * bmp, this can be used to occlude sparse object data 
   * on the next pass.
   */
  struct ShadowcastFnCtx effect_ctx = {
      gfx, x_offset, y_offset, mask
  };
  struct ShadowcastInterface effect_fn = {
      .fn = (ShadowcastFnPtr)shadowcastMarkVisible,
      .ctx = &effect_ctx
  }; 
  shadowcastFOV(effect_fn, camera);

  /* first chunk object pass
   * occludes, renders,  mobiles and items
   */
  GameObject* mobiles = arena->mobiles;
  for(size_t i = 0; i < memSliceSize(mobiles) / sizeof(GameObject); i++){
      if(bitmapGetPx(arena->mobiles_free, i, 0) == 0) continue;    
      GameObject actor = mobiles[i];
      MapPosition pos = actor.type.mob.pos;
      if(bitmapGetPx(mask, pos.x, pos.y) == 1){
          gfxRenderGlyph(gfx, pos.x - x_offset, pos.y - y_offset, actor.tile.unicode, actor.tile.atlas, actor.tile.fg, actor.tile.bg);
      }
  }

  /* portal passes */
  struct MapPortal* portals = arena->portals;
  for(size_t i = 0; i < memSliceSize(portals) / sizeof(struct MapPortal); i++){
      MapPosition src = {0};
      MapPosition dst = {0};
      if(portalGetSrcDst(portals[i], camera, &src, &dst) == 1) continue;
      if(bitmapGetPx(mask, src.x, src.y) == 1){
              gfxRenderGlyph(gfx, src.x - x_offset, src.y - y_offset,
                             13, 2, 
                             4, 4);
      }
  }

  bitmapDestroy(mask , allocator);
  
  return 0;
}
