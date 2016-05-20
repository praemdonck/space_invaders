#include <string.h>
#include "breakout.h"
#include "hardware_driver.h"
#include "vector_memory.h"
#include "assets.h"
#include "sprites.h"
#include "graphical.h"
#include "sound.h"

#define BREAKOUT_ROWS     3
#define BREAKOUT_COLUMNS  6

enum
{
  BRICK_SPRITE,
  BALL_SPRITE,
  PADDLE_SPRITE,
  BREAKOUT_NUM_SPRITES
};

struct
{
  int ball_speed_x;
  int ball_speed_y;
  int paddle_speed_x;
  unsigned int num_bricks;
  bool ball_in_play;
} breakout_data;

VM_ALLOC(breakout_sprites_vector, sizeof(sprite_t), BREAKOUT_ROWS * BREAKOUT_COLUMNS + 2);



void breakout_set_level(void);
sprite_t* brick_init_sprite(int pos_x, int pos_y, const unsigned char* main_bitmap);
sprite_t* paddle_init_sprite( const unsigned char* main_bitmap );
void paddle_tick(sprite_t* sprite);
sprite_t* ball_init_sprite( const unsigned char* main_bitmap );
void breakout_bitmap_draw(const unsigned char* bitmap, int x_pos, int y_pos);

void brick_ball(sprite_t* s1, sprite_t* s2);
void ball_paddle(sprite_t* s1, sprite_t* s2);


sprite_collision_function_t breakout_collision_functions[BREAKOUT_NUM_SPRITES * BREAKOUT_NUM_SPRITES] = {
/*              BRICK       BALL         PADDLE     */
/* BRICK     */ 0         , brick_ball , 0          ,  
/* BALL      */ brick_ball, 0          , ball_paddle,  
/* PADDLE    */ 0         , ball_paddle, 0           }; 


unsigned int breakout_play_level(void)
{
  breakout_set_level();
  paddle_init_sprite(BREAKOUT_PADDLE);
  ball_init_sprite(BREAKOUT_BALL);
  breakout_data.ball_speed_y = -256;
  breakout_data.ball_speed_x = 128;
  breakout_data.ball_in_play = true;
  

  while (1) 
  {
    if (timer_semaphore)
    {
      timer_semaphore = 0;
      sprite_tick_all(&breakout_sprites_vector);
      sprite_collision_detect(&breakout_sprites_vector, breakout_collision_functions, BREAKOUT_NUM_SPRITES );               // Check for collisions

      if (!breakout_data.ball_in_play) return 0;
      if (!breakout_data.num_bricks) return 1;

      graphical_clear_buffer(display_buffer);  // Clear screen buffer
      sprite_draw_all(&breakout_sprites_vector, breakout_bitmap_draw);  // Redraw all sprites
      pcd8544_write_data(&display_data, display_buffer, DISPLAY_MEMORY_SIZE);
    }
  }; 
}

// Wrapper function of graphical_draw_bitmap that is passed to
// the sprite module to redraw the sprites on screen
void breakout_bitmap_draw(const unsigned char* bitmap, int x_pos, int y_pos)
{
  graphical_draw_bitmap(display_buffer, bitmap, x_pos, y_pos, true);
}

void breakout_set_level(void)
{
  size_t i, j;

  vm_release_all(&breakout_sprites_vector);

  breakout_data.num_bricks = 0;
  for (i = 0; i < BREAKOUT_ROWS; i++)
  {
    for (j = 0; j < BREAKOUT_COLUMNS; j++)
    {
      brick_init_sprite(j*14 + 7, i * 7 + 3, BREAKOUT_BRICK);
      breakout_data.num_bricks++;
    }
  }
}




sprite_t* brick_init_sprite(int pos_x, int pos_y, const unsigned char* main_bitmap)
{
  sprite_t* sprite = sprite_activate(&breakout_sprites_vector);
  if (!sprite) return 0;

  memset(sprite, 0, sizeof(sprite_t));

  sprite->type = BRICK_SPRITE;
  sprite->center_pos_x = pos_x << 8;
  sprite->center_pos_y = pos_y << 8;
  sprite->size_x_div_2 = main_bitmap[0] << 7;
  sprite->size_y_div_2 = main_bitmap[1] << 7;
  sprite->bitmap_ptr = main_bitmap;
  sprite->data = 0;
  sprite->tick_function = 0;
    
  return sprite;
}


void paddle_tick(sprite_t* sprite)
{
  int* previous_pos = (int*)&sprite->data;
  sprite->center_pos_x = (adc_in() * ((SCREENW << 8) - (sprite->size_x_div_2 << 1 ))) >> 12;
  sprite->center_pos_x += sprite->size_x_div_2;

  breakout_data.paddle_speed_x = sprite->center_pos_x - *previous_pos;
  *previous_pos = sprite->center_pos_x;
}

void ball_tick(sprite_t* sprite)
{
  sprite->center_pos_x += breakout_data.ball_speed_x;
  sprite->center_pos_y += breakout_data.ball_speed_y;

  if ( (sprite->center_pos_x <= sprite->size_x_div_2 && breakout_data.ball_speed_x < 0) ||
       (sprite->center_pos_x >= ( (SCREENW << 8) - sprite->size_x_div_2 ) && breakout_data.ball_speed_x > 0 ) )
    breakout_data.ball_speed_x = -breakout_data.ball_speed_x;

  if ( sprite->center_pos_y <= sprite->size_y_div_2 && breakout_data.ball_speed_y < 0)
    breakout_data.ball_speed_y = -breakout_data.ball_speed_y;

  if (sprite->center_pos_y >=  (SCREENH << 8) && breakout_data.ball_speed_y > 0 )
  {
    breakout_data.ball_in_play = false;
    sprite_release(&breakout_sprites_vector, sprite);
  }
}

sprite_t* paddle_init_sprite( const unsigned char* main_bitmap )
{
  sprite_t* sprite = sprite_activate(&breakout_sprites_vector);
  if (!sprite) return 0;

  memset(sprite, 0, sizeof(sprite_t));

  sprite->type = PADDLE_SPRITE;
  sprite->center_pos_x = 0;
  sprite->center_pos_y = 46 << 8;
  sprite->size_x_div_2 = main_bitmap[0] << 7;
  sprite->size_y_div_2 = main_bitmap[1] << 7;
  sprite->bitmap_ptr = main_bitmap;
  sprite->data = 0;
  sprite->tick_function = paddle_tick;

  return sprite;
}


sprite_t* ball_init_sprite( const unsigned char* main_bitmap )
{
  sprite_t* sprite = sprite_activate(&breakout_sprites_vector);
  if (!sprite) return 0;

  memset(sprite, 0, sizeof(sprite_t));

  sprite->type = BALL_SPRITE;
  sprite->center_pos_x = 42 << 8;
  sprite->center_pos_y = 40 << 8;
  sprite->size_x_div_2 = main_bitmap[0] << 7;
  sprite->size_y_div_2 = main_bitmap[1] << 7;
  sprite->bitmap_ptr = main_bitmap;
  sprite->data = 0;
  sprite->tick_function = ball_tick;

  return sprite;
}

// brick ball collision handler
void brick_ball(sprite_t* s1, sprite_t* s2)
{
  sprite_t* ball;
  sprite_t* brick;
  unsigned int collision_direction;

  ball = s1->type == BALL_SPRITE ? s1 : s2;
  brick = s1->type == BRICK_SPRITE ? s1 : s2;

  collision_direction = detect_collision(ball, brick);

  if (collision_direction == NORTH_COLLISION ||
      collision_direction == SOUTH_COLLISION )
    breakout_data.ball_speed_y = -breakout_data.ball_speed_y;

  if (collision_direction == EAST_COLLISION ||
      collision_direction == WEST_COLLISION )
    breakout_data.ball_speed_x = -breakout_data.ball_speed_x;


  ball_tick(ball);
  breakout_data.num_bricks--;
  if (breakout_data.num_bricks)
  {
    //sound_play_enemy_killed();
    sound_play_shoot();
  }
  else
  {
    sound_play_big_explosion(); 
  }

  sprite_release(&breakout_sprites_vector, brick);
}

// ball paddle collision handler
void ball_paddle(sprite_t* s1, sprite_t* s2)
{
  sprite_t* ball;
  sprite_t* paddle;
  unsigned int collision_direction;

  ball = s1->type == BALL_SPRITE ? s1 : s2;
  paddle = s1->type == PADDLE_SPRITE ? s1 : s2;

  sound_play_fast_invader(0);

  collision_direction = detect_collision(paddle, ball);

  if (collision_direction == NORTH_COLLISION && breakout_data.ball_speed_y > 0 )
    //|| collision_direction == SOUTH_COLLISION )
  {
    breakout_data.ball_speed_y = -breakout_data.ball_speed_y;
    breakout_data.ball_speed_x += breakout_data.paddle_speed_x / 8;
    
    // Make sure the ball is above the paddle to avoid multiple collition between the ball and the paddle      
    ball->center_pos_y = paddle->center_pos_y - (paddle->size_y_div_2 + ball->size_y_div_2);
  }
    

  if (collision_direction == EAST_COLLISION ||
      collision_direction == WEST_COLLISION )
  {
    breakout_data.ball_in_play = false;
    sprite_release(&breakout_sprites_vector, ball);
  }
}


