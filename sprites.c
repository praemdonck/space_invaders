#include "sprites.h"
#include "graphical.h"


// TEMP
#include "hardware_driver.h"

// Each sprite on the screen has an associated data structure
// with its type, position, size, bitmap, and pointers
// to a tick function and extra data;
//
// Depending on the sprite type, it may be necessary to 
// store extra information specific to that sprite. The 
// main sprite_t has a pointer to link and extra data structure




// Generic sprite related functions

// Get memory for a new sprite
sprite_t* sprite_activate(vm_t* v)
{
  return (sprite_t*)vm_element_get(v);
}

// Release the memory of an unused sprite
void sprite_release(vm_t* v, sprite_t* sprite)
{
  vm_element_release(v, (void*)sprite);
}

// Draw sprite on screen
//void sprite_draw(sprite_t* sprite, sprite_draw_callback draw_function)
//{
//  graphical_draw_bitmap(display_buffer, sprite->bitmap_ptr, 
//                        sprite->center_pos_x >> 8, 
//                        sprite->center_pos_y >> 8, true);
//}

// Draw all active sprites
void sprite_draw_all(vm_t* v, sprite_draw_callback draw_function)
{
  size_t it = 0;
  sprite_t* sprite = (sprite_t*)vm_iterate(v, &it);

  if (!draw_function) return;

  while (sprite)
  {
    draw_function(sprite->bitmap_ptr, 
                  sprite->center_pos_x >> 8, 
                  sprite->center_pos_y >> 8);

    sprite = (sprite_t*)vm_iterate(v, &it);
  }
}

// Execute tick function of all active sprites
void sprite_tick_all(vm_t* v)
{
  size_t it = 0;
  sprite_t* sprite = (sprite_t*)vm_iterate(v, &it);

  while (sprite)
  {
    // check if sprite is active and has a tick function registered
    if (sprite->tick_function) 
      sprite->tick_function(sprite);

    sprite = (sprite_t*)vm_iterate(v, &it);
  }
}

// Detect collision between all sprites
void sprite_collision_detect(vm_t* v, 
                             sprite_collision_function_t* collision_functions, 
                             size_t num_sprites) 
{      
  size_t it_1 = 0, it_2;
  sprite_t *sprite_1, *sprite_2;

  sprite_1 = (sprite_t*)vm_iterate(v, &it_1);


  while (sprite_1)
  {
    it_2 = it_1;
    sprite_2 = (sprite_t*)vm_iterate(v, &it_2);
    while (sprite_2)
    {
      sprite_collision_function_t col_func= 0;
      // Check if there is a collision handler function
      // for sprites sprite_1 and sprite_2
      if (sprite_1->type < num_sprites && sprite_2->type < num_sprites)
      {
        col_func = collision_functions[sprite_1->type * num_sprites + sprite_2->type];
      }

      // Only if a collision handler function exists, detect the collision
      if (col_func && detect_collision(sprite_1, sprite_2))
      {
        // If a collision was detected, execute collision handler
        col_func(sprite_1, sprite_2);
      }
      sprite_2 = (sprite_t*)vm_iterate(v, &it_2);
    }
    sprite_1 = (sprite_t*)vm_iterate(v, &it_1);
  }
}


// Collision Detection function.
// If a collision is detected, the function returns 
// the collision side with respect of s1
#define abs(a, b)   (((a) > (b))  ?  ((a) - (b)) : ((b) - (a)))
collision_t detect_collision(sprite_t* s1, sprite_t* s2)
{
  int delta_x = abs(s1->center_pos_x, s2->center_pos_x);
  int delta_y = abs(s1->center_pos_y, s2->center_pos_y);

  int collision_threshold_x = s1->size_x_div_2 + s2->size_x_div_2;
  int collision_threshold_y = s1->size_y_div_2 + s2->size_y_div_2;
    
  if (delta_x <= collision_threshold_x &&
      delta_y <= collision_threshold_y)
  {
    // A collision was detected, now try to work out on which side
    if ((collision_threshold_x - delta_x) < (collision_threshold_y - delta_y))
    {
      // EAST or WEST collision
      return   s1->center_pos_x < s2->center_pos_x ? EAST_COLLISION : WEST_COLLISION;
    }
    else
    {
      // NORTH or SOUTH collision
      return   s1->center_pos_y > s2->center_pos_y ? NORTH_COLLISION : SOUTH_COLLISION;
    }
  }
  else
    return NO_COLLISION;
}

