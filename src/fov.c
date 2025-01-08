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
    int dx;
    int dy;
};

typedef void (*ShadowcastFnPtr)(void*, MapPosition);
struct ShadowcastEffect{
    ShadowcastFnPtr fn;
    void* ctx;
};

// TODO, static module based stack

/* turns a shadowcast coordinate (row_depth,row_col) 
 * that is relative to a single origin point into 
 * a chunk-local position (x, y) we can use to query
 * the world arena
 */
void shadowmaskToWorldspace(int cardinal, int depth, int col, MapPosition camera, MapPosition* out) {
    switch(cardinal) {
        case NORTH:
            out->x = camera.x + col;
            out->y = camera.y - depth;
            break;
        case SOUTH:
            out->x = camera.x + col;
            out->y = camera.y + depth;
            break;
        case EAST:
            out->x = camera.x + depth;
            out->y = camera.y + col;
            break;
        case WEST:
            out->x = camera.x - depth;
            out->y = camera.y + col;
            break;
    }
}
/* reverse of shadowmaskToWorldspace */
void worldspaceToShadowmask(MapPosition in, MapPosition camera, int* cardinal_out, int* depth_out, int* col_out) {
    int dx = in.x - camera.x;
    int dy = in.y - camera.y;

    if (abs(dx) > abs(dy)) {
        // East or West
        *col_out = dy;
        *depth_out = abs(dx);
        if (dx > 0) {
            *cardinal_out = EAST;
        } else {
            *cardinal_out = WEST;
        }
    } else {
        // North or South
        *col_out = dx;
        *depth_out = abs(dy);       
        if (dy > 0) {
            *cardinal_out = SOUTH;
        } else {
            *cardinal_out = NORTH;
        }
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

void shadowcastRenderVisible(struct ShadowcastFnCtx* ctx, MapPosition pos) {
 
    int scr_x = pos.x - ctx->dx;
    int scr_y = pos.y - ctx->dy;
    struct GameObjectTile t = terraGetTile(pos);
    gfxRenderGlyph(ctx->gfx, scr_x, scr_y, 
                   t.unicode, t.atlas, t.fg, t.bg);
}

static void shadowcastScanRow( MapPosition camera, Row current_row, Bitmap* dst_mask, struct ShadowcastEffect effect ){

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
    MapPosition current_pos = camera;
    shadowmaskToWorldspace(current_row.cardinal, 
                           current_row.depth, col,
                           camera, &current_pos);

    uint8_t is_wall = terraDoesBlockSight(current_pos);
    
    if(is_wall || isSymmetric(&current_row, col)) {
      if(relativeDistance(current_row.depth, col) < SHADOWCAST_MAX){
          effect.fn(effect.ctx, current_pos);
          bitmapSetPx(dst_mask, current_pos.x, current_pos.y, 1);
      }
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
      shadowcastScanRow(camera, next_row, dst_mask, effect);
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
    shadowcastScanRow(camera, next_row, dst_mask, effect);
  }
  
}

void shadowcastFOV(MapPosition camera, Bitmap* dst_mask, 
                   struct ShadowcastEffect effect){

  effect.fn(effect.ctx, camera);
  bitmapSetPx(dst_mask, camera.x, camera.y, 1);
  for(int cardinal = 0; cardinal < 4; cardinal++) {
    
    Row first_row = {
      .cardinal = cardinal,
      .depth = 1,
      .start_slope = fractionNew(-1, 1),
      .end_slope = fractionNew(1, 1)
    };
    shadowcastScanRow(camera, first_row, dst_mask, effect);
    
  }
}

void shadowcastPortal(MapPosition camera, struct MapPortal portal, Bitmap* dst_mask, struct ShadowcastEffect effect){

    effect.fn(effect.ctx, portal.dst);
    
    int cardinal, depth, col;
    worldspaceToShadowmask(portal.src, camera, &cardinal, &depth, &col);
    Row first_row = {
        .cardinal = cardinal,
        .depth = 1,
        .start_slope = slope(depth, col ),
        .end_slope = slope(depth, col + 1),
    };
    shadowcastScanRow(portal.dst, first_row, dst_mask, effect);
}

int cameraDrawWorld(Gfx gfx, MapPosition camera, AllocatorInterface allocator){

  gfxClear(gfx);
  int scr_width = gfxGetScreenWidth(gfx);
  int scr_height = gfxGetScreenHeight(gfx);

  Bitmap* mask = bitmapCreate(scr_width, scr_height, allocator);
  int dx = camera.x - (scr_width / 2);
  int dy = camera.y - (scr_height / 2);

  struct ShadowcastFnCtx effect_ctx = {
      gfx, dx, dy
  };
  struct ShadowcastEffect effect = {
      .fn = (ShadowcastFnPtr)shadowcastRenderVisible,
      .ctx = &effect_ctx
  }; 

  /* First chunk rendering pass
   * draws terrain to the screen and a visibility mask
   * bmp, this can be used to occlude sparse object data 
   * on the next pass.
   */
  shadowcastFOV(camera, mask, effect);
  struct WorldArena *arena = camera.chunk_ptr->ptr_to_arena;

  /* first chunk object pass
   * occludes, renders,  mobiles and items
   */
  GameObject* mobiles = arena->mobiles;
  for(size_t i = 0; i < memSliceSize(mobiles) / sizeof(GameObject); i++){
      if(bitmapGetPx(arena->mobiles_free, i, 0) == 0) continue;
      GameObject actor = mobiles[i];
      MapPosition pos = actor.type.mob.pos;
      if(bitmapGetPx(mask, pos.x, pos.y) == 1){
          gfxRenderGlyph(gfx, pos.x - dx, pos.y - dy, 
                         actor.tile.unicode, actor.tile.atlas, actor.tile.fg, actor.tile.bg);
      }
  }

  /* portal passes */
  struct MapPortal* portals = arena->portals;
  for(size_t i = 0; i < memSliceSize(portals) / sizeof(struct MapPortal); i++){
      struct MapPortal port = portals[i];
      if(port.src.chunk_ptr == NULL) continue;
      if(bitmapGetPx(mask, port.src.x, port.src.y) == 1){
          effect_ctx.dx = dx + port.dst.x - port.src.x;
          effect_ctx.dy = dy + port.dst.y - port.src.y;
          shadowcastPortal(camera, port, NULL, effect);
      }

  }

  bitmapDestroy(mask , allocator);
  return 0;
}
