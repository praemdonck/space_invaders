#ifndef _sprites_h
#define _sprites_h

#include <stdbool.h>
#include "vector_memory.h"

// Collision Detection function.
// This function was modified to return the collision 
// side with respect of s1, so it could be used by
// breakout game
typedef enum {
  NO_COLLISION,
  NORTH_COLLISION,
  EAST_COLLISION,
  SOUTH_COLLISION,
  WEST_COLLISION
} collision_t;


// Structure defining sprite
// In the sprite world the unit of distances
// are in pixels * 256 ie pixel at position (2,2) would be (512,512).
// This gives more control on the speed of the sprites.
// for example if a sprite moves at 64 units/refresh cycle
// it would actually move 1 pixel every 4 refresh cycles.
typedef struct sprite
{
  int type;
  // This represet the position of the center of the sprite
  int center_pos_x; 
  int center_pos_y;
  // This represent the distance from the center of the sprite
  // to the left or right edge. 
  int size_x_div_2; // size of the sprite * 256 / 2 used by the collission engine
  // This represent the distance from the center of the sprite
  // to the top or bottom edge. 
  int size_y_div_2; 
  // Pointer to the bitmap for the sprite
  const unsigned char* bitmap_ptr;
  // Pointer to extra data
  void* data;
  // Pointer to a tick function that gets executed
  // on every cycle. This is normally used to move
  // the sprite  
  void(*tick_function)(struct sprite*);
} sprite_t;

typedef void(*sprite_collision_function_t)(sprite_t*, sprite_t*);
typedef void(*sprite_draw_callback)(const unsigned char* bitmap, int x_pos, int y_pos);

// Sprite Related functions
sprite_t* sprite_activate(vm_t* v);
void sprite_release(vm_t* v, sprite_t* sprite);
void sprite_draw_all(vm_t* v, sprite_draw_callback draw_function);
void sprite_tick_all(vm_t* v);
//void sprite_collision_detect(vm_t* v);


collision_t detect_collision(sprite_t* s1, sprite_t* s2);

void sprite_collision_detect(vm_t* v, sprite_collision_function_t* collision_functions_table, size_t num_sprites);

#endif

