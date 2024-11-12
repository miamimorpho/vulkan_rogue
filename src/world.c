#include "world.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

EntityArray entityArrayMalloc(size_t capacity){
  return (EntityArray){
    .capacity = capacity,
    .count = 0,
    .data = malloc(sizeof(Entity) * capacity)
  };
}

Entity* entityInit(GameWorld* world, unsigned int entity_index){

  if(entity_index > world->actors_count){
    return NULL;
  }
  
  Entity* e = &world->actors[entity_index];
  e->is_init = 1;
  e->x = 0;
  e->y = 0;
  e->uv = 1024;
  e->fg = 15;
  e->bg = 0;
  e->collide = 1;
  e->inventory = entityArrayMalloc(10);
    
  return e;
}

typedef struct {
    int num;
    int den;
} Fraction;

// Row structure
typedef struct {
    int depth;
    Fraction start_slope;
    Fraction end_slope;
} Row;

// Cardinal directions
enum {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};

// Helper functions for fractions
Fraction fractionNew(int num, int den) {
    Fraction f = {num, den};
    return f;
}

bool fractionCompare(Fraction small, Fraction large){
  return(small.num * large.den <= large.num * small.den);
}

// Calculate slope as a fraction
Fraction slope(int row_depth, int col) {
    return fractionNew(2 * col - 1, 2 * row_depth);
}

// Transform coordinates based on quadrant
void transform_coords(int cardinal, int ox, int oy, int row, int col, int* out_x, int* out_y) {
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

// Helper functions for rounding
int round_ties_up(double n) {
      return (int)floor(n + 0.5);
}

int round_ties_down(double n) {
   return (int)ceil(n - 0.5);

}

bool is_symmetric(Row* row, int col) {
    // Multiply both sides by depth to avoid creating new fractions
    int depth_times_start = row->depth * row->start_slope.num;
    int start_check = col * row->start_slope.den;
    
    int depth_times_end = row->depth * row->end_slope.num;
    int end_check = col * row->end_slope.den;
    
    return (start_check >= depth_times_start && end_check <= depth_times_end);
}


// Example usage:
bool is_blocking(GameWorldTerrain terrain, int x, int y) {
  Entity tile = mapGetTile(terrain, x, y);
  if(tile.blocks_sight == 1)
    return true;

  return false;
}

void mark_visible(GameWorldTerrain terrain, EntityArray* to_draw_buffer, int x, int y) {
  Entity tile = mapGetTile(terrain, x, y);
  if(to_draw_buffer->count +1 > to_draw_buffer->capacity){
    printf("FATAL: out of memory\n");
  }
  //printf("(%d %d)\n", x, y);
  to_draw_buffer->data[to_draw_buffer->count++] = tile;
  // Implement your visibility marking here
  //printf("Marked visible: (%d, %d)\n", x, y);
}

// Main FOV computation function
void compute_fov(GameWorldTerrain terrain, EntityArray* to_draw_buffer, int origin_x, int origin_y)
{    
  mark_visible(terrain, to_draw_buffer, origin_x, origin_y);
  
  for(int cardinal = 0; cardinal < 4; cardinal++) {
   
    // Initialize first row
    Row first_row = {
      .depth = 1,
      .start_slope = fractionNew(-1, 1),
      .end_slope = fractionNew(1, 1)
    };
    
    Row* row_stack = malloc(sizeof(Row) * 1000); // Adjust size as needed
    int stack_size = 1;
    row_stack[0] = first_row;
    
    while(stack_size > 0) {
      Row current_row = row_stack[--stack_size];

      if (fractionCompare(current_row.end_slope, current_row.start_slope)) {
	continue;
      }
      
      // int * fraction
      int min_col = round_ties_up( (double)
	(current_row.depth * current_row.start_slope.num)
	/ (double)current_row.start_slope.den);
      int max_col = round_ties_down(
	(double)(current_row.depth * current_row.end_slope.num)
	/ (double)current_row.end_slope.den);

      
      bool prev_was_wall = false;
      bool prev_was_floor = false;      

      // Scan through tiles in the row
      for(int col = min_col; col <= max_col; col++) {
	int x, y;
	transform_coords(cardinal, origin_x, origin_y, 
			 current_row.depth, col, &x, &y);
	
	bool is_wall = is_blocking(terrain, x, y);
        
	if(is_wall || is_symmetric(&current_row, col)) {
	  mark_visible(terrain, to_draw_buffer, x, y);
	  
	}
        
	if(prev_was_wall && !is_wall) {
	  current_row.start_slope = slope(current_row.depth, col);
	}
        
	if(!prev_was_wall && is_wall && stack_size < 999) {
	  Row next_row = {
	    .depth = current_row.depth + 1,
	    .start_slope = current_row.start_slope,
	    .end_slope = slope(current_row.depth, col)
	  };
	  row_stack[stack_size++] = next_row;
	}
        
	prev_was_wall = is_wall;
	prev_was_floor = !is_wall;
      }// end of row scanning
      
      // If we hit floor at the end, scan next row
      if(prev_was_floor && stack_size < 999) {
	Row next_row = {
	  .depth = current_row.depth + 1,
	  .start_slope = current_row.start_slope,
	  .end_slope = current_row.end_slope
	};
	row_stack[stack_size++] = next_row;
      }
    }
    
    free(row_stack);
  }
}

int worldDraw(Gfx gfx, GameWorld world, Entity camera){

  gfxClear(gfx);
  
  int x_offset = (ASCII_SCREEN_WIDTH / 2);
  int y_offset = (ASCII_SCREEN_HEIGHT / 2);

  EntityArray to_draw_buffer = entityArrayMalloc(5000);
  compute_fov(world.terrain, &to_draw_buffer, camera.x, camera.y);
  for(int i = 0; i < to_draw_buffer.count; i++){
    Entity tile = to_draw_buffer.data[i];
    int tile_x = tile.x - camera.x + x_offset;
    int tile_y = tile.y - camera.y + y_offset;;
    gfxAddCh(gfx, tile_x, tile_y, tile.uv, DRAW_TEXTURE_INDEX, tile.fg, tile.bg);
  }
  free(to_draw_buffer.data);
  
  /*
  for(int y = 0; y < ASCII_SCREEN_HEIGHT; y++){
    for(int x = 0; x < ASCII_SCREEN_WIDTH; x++){
      Entity tile = mapGetTile(world.terrain,
			       camera.pos.x - x_offset +x,
			       camera.pos.y - y_offset +y);
      gfxAddCh(gfx, x, y,
	       tile.uv, DRAW_TEXTURE_INDEX,
	       tile.fg, tile.bg);
    }
  }
  */
  
  // render list of entities (actors)
  for(unsigned int i = 0; i < world.actors_count; i++){
    Entity actor = world.actors[i];
    if(actor.is_init == 1){
      gfxAddCh(gfx, x_offset, y_offset,
	       actor.uv, DRAW_TEXTURE_INDEX,
	       actor.fg, actor.bg);
    }
  }
  return 0;
}

int worldInit(GameWorld* w, int width, int height){
  w->terrain.width = width;
  w->terrain.size = width * height;
  w->terrain.tiles = (Entity*)malloc(w->terrain.size * sizeof(Entity));

  Entity air = {
    .uv = 0,
    .fg = 15,
    .bg = 3,
    .collide = 0,
    .blocks_sight = 0
  };
  
  for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      mapPutTile(&w->terrain, air, x, y);
    }
  }
  int actor_c = 8;
  w->actors = malloc(actor_c * sizeof(Entity));
  w->actors_count = actor_c;
  for(int i = 0; i < actor_c; i++){
    w->actors[i].is_init = 0;
  }
  
  return 0;
}

Entity mapGetTile(GameWorldTerrain terrain, int x, int y){
  Entity null_ent = {
    .uv = 370,
    .fg = 3,
    .bg = 0,
    .collide = 0,
    .blocks_sight = 1,
    .x = x,
    .y = y
  };
  if(x >= terrain.width) return null_ent;
  if(x < 0) return null_ent;
  if(y < 0) return null_ent;
  
  int offset = (y * terrain.width) + x;
  if(offset >= terrain.size) return null_ent;
  return terrain.tiles[offset];
}

int mapPutTile(GameWorldTerrain* terrain, Entity entity, int x, int y){

  if(x >= terrain->width) return 1;
  if(x < 0) return 1;
  if(y < 0) return 1;
  int offset = (y * terrain->width) + x;  
  if(offset >= terrain->size ) return 1;
  
  entity.x = x;
  entity.y = y;
  terrain->tiles[offset] = entity;
  return 0;
}

Entity* entityGet(GameWorld* world, unsigned int entity_index){
  if(entity_index > world->actors_count){
    return NULL;
  }
  return &world->actors[entity_index];
}
