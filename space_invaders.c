#include <string.h>
#include "space_invaders.h"
#include "graphical.h"
#include "assets.h"
#include "pcd8544.h"
#include "hardware_driver.h"
#include "sound.h"
#include "sprites.h"
#include "Random.h"



// Enum with Sprite types
enum
{
  ENEMY,
  SHIP,
  BUNKER,
  MISSILE,
  LASER,
  EXPLOSION,
  NUM_SPRITES_TYPES
};

typedef enum
{
  RIGHT,
  DOWN_RIGHT,
  LEFT,
  DOWN_LEFT
} enemy_movement_sequence_e;


// Extra data for enemy sprites
typedef struct
{
  const unsigned char* alternate_bitmap;
  unsigned int bitmap_alternate_rate;
  unsigned int bitmap_alternate_rate_counter;
  int value;
} enemy_data_t;

// Extra data for weapon sprites
typedef struct
{
  const unsigned char* alternate_bitmap;
  unsigned int bitmap_alternate_rate;
  unsigned int bitmap_alternate_rate_counter;
  int weapon_speed_x;
  int weapon_speed_y;
} weapon_data_t;

// Extra data for explosion sprites
typedef struct
{
  const unsigned char* alternate_bitmap;
  unsigned int lifetime;
} explosion_data_t;


// Data structure to keep track
// of the high level data
typedef struct
{
  // Current score
  int score;
  // Number of remaining lives
  int lives;

  // Enemy speed used to move
  // enemies in sync
  int enemy_speed_x;
  int enemy_speed_y;
  // List of enemies still active
  sprite_t* enemies_active[MAX_ENEMIES];
  // Number of enemies alive
  int enemies_active_count;
  // mothership still alive
  bool cannon_alive;
  bool ufo_passed;

  // Current stage in the movement sequence
  // of the enemies ie left, down left, right or down right
  enemy_movement_sequence_e enemy_movement;

  // Position of the mothership
  int ship_cannon_position_x;
  int ship_cannon_position_y;

  // Input related variables
  unsigned int pot_position;
  bool button_a_previous_state;
  bool button_b_previous_state;
  bool button_a_pressed;
  bool button_b_pressed;

  unsigned int tempo_counter;
  unsigned int tempo_sequence;
  unsigned int last_shot_time;

  bool special_mode;
} space_invaders_data_t;

// Data used to control level
// parameters
typedef struct
{
  int speed_offset;
  int shooting_delay;
  int shooting_probability_offset;
  int missile_speed;
  int laser_speed;
} space_invaders_level_data_t;

// Space invader top level functions
void space_invaders_reset_enemy_position(void);
void space_invaders_tick(bool shoot_enable);
void space_invaders_handle_inputs(void);
void space_invaders_draw_lives(unsigned int lives);


// Enemy Related functions
void enemy_data_vector_init(void);

sprite_t* enemy_init_sprite(int pos_x, int pos_y,
                            const unsigned char* main_bitmap,
                            const unsigned char* alternate_bitmap,
                            void(*tick_function)(sprite_t*), int value);

void enemy_tick(sprite_t* sprite);
void ufo_tick(sprite_t* sprite);
void enemy_release_sprite(sprite_t* sprite);
bool enemy_random_shoot(void);


// Bunker Related functions
void bunker_degrade(sprite_t* sprite);
sprite_t* bunker_init_sprite(int pos_x, int pos_y,
                            const unsigned char* bitmap);


// Ship Related functions
sprite_t* ship_init_sprite( const unsigned char* main_bitmap );

// Weapon Related functions
void weapon_tick(sprite_t* sprite);
void weapon_release_sprite(sprite_t* sprite);
void weapon_release_all(void);
void weapon_data_vector_init(void);
sprite_t* weapon_init_sprite(int pos_x, int pos_y, int weapon_type,
                             const unsigned char* main_bitmap,
                             const unsigned char* alternate_bitmap,
                             int velocity_x, int velocity_y);



// Explosion Related functions
void explosion_tick(sprite_t* sprite);
void explosion_release_sprite(sprite_t* sprite);
void explosion_data_vector_init(void);
sprite_t* explosion_init_sprite(int pos_x, int pos_y,
                                const unsigned char* main_bitmap,
                                const unsigned char* alternate_bitmap,
                                unsigned int lifetime);


// Data structures containing info shared
// accros multiple games modules
space_invaders_data_t space_invader_data;
space_invaders_level_data_t space_invaders_level_data;

// Collision detection and handling functions

void enemy_laser(sprite_t* s1, sprite_t* s2);
void enemy_ship(sprite_t* s1, sprite_t* s2);
void enemy_bunker(sprite_t* s1, sprite_t* s2);
void missile_laser(sprite_t* s1, sprite_t* s2);
void bunker_missile(sprite_t* s1, sprite_t* s2);
void bunker_laser(sprite_t* s1, sprite_t* s2);
void ship_missile(sprite_t* s1, sprite_t* s2);

// This table determines which handler need to be executed when a collision between 2 sprites is detected
sprite_collision_function_t sprite_collision_functions[NUM_SPRITES_TYPES * NUM_SPRITES_TYPES] = {
/*              ENEMY         SHIP          BUNKER          MISSILE         LASER          EXPLOSION */
/* ENEMY     */ 0           , enemy_ship  , enemy_bunker  , 0             , enemy_laser  , 0,
/* SHIP      */ enemy_ship  , 0           , 0             , ship_missile  , 0            , 0,
/* BUNKER    */ enemy_bunker, 0           , 0             , bunker_missile, bunker_laser , 0,
/* MISSILE   */ 0           , ship_missile, bunker_missile, 0             , missile_laser, 0,
/* LASER     */ enemy_laser , 0           , bunker_laser  , missile_laser , 0            , 0,
/* EXPLOSION */ 0           , 0           , 0             , 0             , 0            , 0 };



// Memory for the sprites
VM_ALLOC(sprites_vector, sizeof(sprite_t), MAX_SPRITES);

// Memory for aditional data for enemies sprites
VM_ALLOC(enemy_data_vector, sizeof(enemy_data_t), MAX_ENEMIES);

// Memory for aditional data for weapons sprites
VM_ALLOC(weapon_data_vector, sizeof(weapon_data_t), MAX_WEAPONS);

// Memory for aditional data for explosions sprites
VM_ALLOC(explosion_data_vector, sizeof(explosion_data_t), MAX_EXPLOSIONS);


typedef struct
{
  unsigned int speed_x;
  unsigned int speed_y;
  unsigned int tempo;
  unsigned int shooting_p;
} tempo_t;

// This table defines the base speed, the sound tempo and the
// shooting probabilities in function of the number of enemies
// remaining on screen
const tempo_t tempo_data[] = { /* Speed x, Speed y, Tempo, Shoot P */
/*  1 Enemy   */               {  400,     256,     2 ,    12},
/*  2 Enemies */               {  350,     256,     3 ,    7},
/*  3 Enemies */               {  300,     256,     3 ,    6},
/*  4 Enemies */               {  180,     256,     4 ,    5},
/*  5 Enemies */               {  157,     256,     5 ,    5},
/*  6 Enemies */               {  146,     256,     10,    4},
/*  7 Enemies */               {  136,     256,     10,    4},
/*  8 Enemies */               {  125,     256,     15,    4},
/*  9 Enemies */               {  114,     256,     15,    4},
/* 10 Enemies */               {  104,     256,     20,    4},
/* 11 Enemies */               {  93 ,     256,     20,    4},
/* 12 Enemies */               {  82 ,     256,     25,    4},
/* 13 Enemies */               {  71 ,     256,     25,    4},
/* 14 Enemies */               {  61 ,     256,     30,    4},
/* 15 Enemies */               {  50 ,     256,     30,    4} };




// Wrapper function of graphical_draw_bitmap that is passed to
// the sprite module to redraw the sprites on screen
void space_invaders_bitmap_draw(const unsigned char* bitmap, int x_pos, int y_pos)
{
  graphical_draw_bitmap(display_buffer, bitmap, x_pos, y_pos, true);
}

// Show Space Invaders splash screen
void space_invaders_show_splash(unsigned int delay)
{
  graphical_clear_buffer(display_buffer);
  graphical_draw_bitmap(display_buffer, SPACE_INVADER_SPLASH, 42, 24, true);
  pcd8544_write_data(&display_data, display_buffer, DISPLAY_MEMORY_SIZE);
  delay_100_ms(delay);
}

int space_invaders_get_score(void)
{
 return space_invader_data.score;
}

void space_invaders_increment_score(int n)
{
  space_invader_data.lives += n;
}

void space_invaders_set_sm(bool val)
{
  space_invader_data.special_mode = val;
}

// Play a level of space invaders
// The function returns 1 if all enemies were killed
// or 0 if the player lost
unsigned int space_invaders_play_level(void)
{
  int tempo_counter = 0, tempo;
  int active_enemies;
  unsigned int terminate_counter = 0;





  for (;;)
  {
    if (timer_semaphore)
    {
      timer_semaphore = 0;

      // Update inputs
      space_invaders_handle_inputs();

      // blink red led based on tempo
      active_enemies = space_invader_data.enemies_active_count;
      if (active_enemies > 0)
        tempo = tempo_data[active_enemies - 1].tempo;

      RED_LED = 0;
      tempo_counter++;
      if (tempo_counter > tempo)
      {
        tempo_counter = 0;
        RED_LED = 1;
      }

      // Check if the level terminated, the terminate counter
      // keep ticking the game for another 0.5 second after the
      // cannon is dead or all the enemies are dead
      if (space_invader_data.cannon_alive == false ||
          space_invader_data.enemies_active_count == 0)
        terminate_counter++;

      if (space_invader_data.cannon_alive == false && terminate_counter > 15)
      {
        // Cannon was shot, check if there are more lives
        space_invader_data.lives--;
        if (space_invader_data.lives > 0)
        {
          // Remove all weapons before starting a new life
          weapon_release_all();
          // Reinitialise the cannon
          ship_init_sprite(PLAYER_SHIP);
          space_invader_data.cannon_alive = true;
          terminate_counter = 0;
        }
        else
        {
           // No more live, games over
           return 0;
        }
      }

      // Return 1 indicating we successfuly killed all enemies
      if (space_invader_data.enemies_active_count == 0 && terminate_counter > 15) return 1;


      space_invaders_tick(!terminate_counter); // Tick main space invader handler
      sprite_tick_all(&sprites_vector);         // Tick the sprites to make the move
      sprite_collision_detect(&sprites_vector, sprite_collision_functions, NUM_SPRITES_TYPES );               // Check for collisions

      graphical_clear_buffer(display_buffer);  // Clear screen buffer
      graphical_draw_number(display_buffer, space_invader_data.score, 5, 0); // Draw score
      space_invaders_draw_lives(space_invader_data.lives);           // Draw lives
      sprite_draw_all(&sprites_vector, space_invaders_bitmap_draw);                                             // Redraw all sprites
      // Refresh display
      pcd8544_write_data(&display_data, display_buffer, DISPLAY_MEMORY_SIZE);
    }
  }
}

// Initialise space_invaders_data
void space_invaders_init(void)
{
  space_invader_data.score = 0;
  space_invader_data.lives = LIVES;
  space_invader_data.enemy_speed_x = 0;
  space_invader_data.enemy_speed_y = 0;
  space_invader_data.enemy_movement = RIGHT;
  space_invader_data.tempo_sequence = 0;
  space_invader_data.tempo_counter = 0;
  space_invader_data.last_shot_time = timer_count;
}

// Configure level specific data and create and 
// position all necessary sprites
void space_invaders_set_level(unsigned int level)
{
  size_t i, temp_index;
  unsigned int temp;
  sprite_t* sp_ptr;

  vm_release_all(&sprites_vector);
  vm_release_all(&enemy_data_vector);
  vm_release_all(&weapon_data_vector);
  vm_release_all(&explosion_data_vector);

  // Adjust game parameters according to level
  // Increase the speed offset of the enemies
  space_invaders_level_data.speed_offset = level * 5;
  // Increase the shooting probability of the enemies
  space_invaders_level_data.shooting_probability_offset = level;
  // Reduce the max shooting cadence of the cannon by increasing
  // the min delay between shots
  temp = level;
  if (space_invader_data.special_mode)
    space_invaders_level_data.shooting_delay = 0;
  else
    space_invaders_level_data.shooting_delay = temp <= 20 ? temp : 20;
  
  // Increase enemy missile speed
  temp = MISSILE_SPEED + level * 8;
  space_invaders_level_data.missile_speed = temp < LASER_SPEED / 2 ? temp : LASER_SPEED / 2;
  // Cannon Laser speed remains constant through levels
  space_invaders_level_data.laser_speed = LASER_SPEED;

  space_invader_data.ufo_passed = false;
  space_invader_data.cannon_alive = true;
  space_invader_data.enemies_active_count = 0;

  for (i = 0; i < MAX_ENEMIES; i++)
    space_invader_data.enemies_active[i] = 0;

  // Position all enemies
  #define ENEMY_WIDTH  13
  temp_index = 0;
  // Position top row enemies
  for (i = 1; i <= ENEMY_COLUMNS; i++)
  {
    sp_ptr = enemy_init_sprite(i * ENEMY_WIDTH + 7, 5, SMALL_ENEMY_30_A,
                               SMALL_ENEMY_30_B,
                               enemy_tick, 30);

    space_invader_data.enemies_active[temp_index++] = sp_ptr;
    space_invader_data.enemies_active_count++;
  }

  // Position middle row enemies
  for (i = 1; i <= ENEMY_COLUMNS; i++)
  {
    sp_ptr = enemy_init_sprite(i * ENEMY_WIDTH + 7, 15, SMALL_ENEMY_20_A,
                               SMALL_ENEMY_20_B,
                               enemy_tick, 20);

    space_invader_data.enemies_active[temp_index++] = sp_ptr;
    space_invader_data.enemies_active_count++;
  }

  // Position bottom row enemies
  for (i = 1; i <= ENEMY_COLUMNS; i++)
  {
    sp_ptr = enemy_init_sprite(i * ENEMY_WIDTH + 7, 25, SMALL_ENEMY_10_A,
                               SMALL_ENEMY_10_B,
                               enemy_tick, 10);

    space_invader_data.enemies_active[temp_index++] = sp_ptr;
    space_invader_data.enemies_active_count++;
  }

  // init bunkers and cannon
  bunker_init_sprite(SCREENW / 4, 38, BUNKER_0);
  bunker_init_sprite((SCREENW * 3) / 4, 38, BUNKER_0);
  ship_init_sprite(PLAYER_SHIP);
}

// Move enemies back to their original rows
void space_invaders_reset_enemy_position(void)
{
  size_t i;
  for (i = 0; i < MAX_ENEMIES; i++)
  {
    int pos_y = (5 + (i / ENEMY_COLUMNS) * 10) * 256;
    if (space_invader_data.enemies_active[i])
    {
      space_invader_data.enemies_active[i]->center_pos_y = pos_y;
    }
  }
}

// Get the topmost position of the enemy block
int space_invaders_get_topmost_enemy_position(void)
{
  int topmost = SCREENH * 256;
  int i;
  for (i = 0; i < MAX_ENEMIES; i++)
  {
    if (space_invader_data.enemies_active[i] &&
        space_invader_data.enemies_active[i]->center_pos_y < topmost)
    {
      topmost = space_invader_data.enemies_active[i]->center_pos_y;
    }
  }
  return topmost;
}

// Get the leftmost position of the enemy block
int space_invaders_get_leftmost_enemy_position(void)
{
  int leftmost = SCREENW << 8;
  int i;
  for (i = 0; i < MAX_ENEMIES; i++)
  {
    if (space_invader_data.enemies_active[i] &&
        space_invader_data.enemies_active[i]->center_pos_x < leftmost)
    {
      leftmost = space_invader_data.enemies_active[i]->center_pos_x;
    }
  }
  return leftmost;
}

// Get the rightmost position of the enemy block
int space_invaders_get_rightmost_enemy_position(void)
{
  int rightmost = 0;
  int i;
  for (i = 0; i < MAX_ENEMIES; i++)
  {
    if (space_invader_data.enemies_active[i] &&
        space_invader_data.enemies_active[i]->center_pos_x > rightmost)
    {
      rightmost = space_invader_data.enemies_active[i]->center_pos_x;
    }
  }
  return rightmost;
}

// This function determines the enemies movement direction
// and speed.
// It also generates Lasers and missiles based on input and 
// random rules and determines when the mothership should pass
void space_invaders_tick(bool shoot_enable)
{
  int enemy_block_position;
  unsigned int enemy_speed_x = tempo_data[space_invader_data.enemies_active_count - 1].speed_x;
  unsigned int enemy_speed_y = tempo_data[space_invader_data.enemies_active_count - 1].speed_y;


  enemy_speed_x += (enemy_speed_x >> 5) * space_invaders_level_data.speed_offset;

  if (enemy_speed_x > MAX_SPEED_CAP) enemy_speed_x = MAX_SPEED_CAP;

  // Calculate movement of the enemies
  switch (space_invader_data.enemy_movement)
  {
    case RIGHT:
      enemy_block_position = space_invaders_get_rightmost_enemy_position();
      if (enemy_block_position > ((SCREENW - 7) << 8))
      {
        space_invader_data.enemy_movement = DOWN_RIGHT;
        space_invader_data.enemy_speed_x = 0;
        space_invader_data.enemy_speed_y = enemy_speed_y;
      }
      else
      {
        space_invader_data.enemy_speed_x = enemy_speed_x;
        space_invader_data.enemy_speed_y = 0;
      }
      break;

    case DOWN_RIGHT:
      space_invader_data.enemy_movement = LEFT;
      space_invader_data.enemy_speed_x = -enemy_speed_x;
      space_invader_data.enemy_speed_y = 0;
      break;

    case LEFT:
      enemy_block_position = space_invaders_get_leftmost_enemy_position();
      if (enemy_block_position < (7 << 8))
      {
        space_invader_data.enemy_movement = DOWN_LEFT;
        space_invader_data.enemy_speed_x = 0;
        space_invader_data.enemy_speed_y = enemy_speed_y;
      }
      else
      {
        space_invader_data.enemy_speed_x = -enemy_speed_x;
        space_invader_data.enemy_speed_y = 0;
      }
      break;

    case DOWN_LEFT:
      space_invader_data.enemy_movement = RIGHT;
      space_invader_data.enemy_speed_x = enemy_speed_x;
      space_invader_data.enemy_speed_y = 0;
  }

  // Check if player is allowed to shoot
  if ((timer_count - space_invader_data.last_shot_time >  space_invaders_level_data.shooting_delay) &&
       shoot_enable)
  {
    // show green led when shooting is available
    GREEN_LED = 1;
    if (space_invader_data.button_a_pressed &&
        weapon_init_sprite(space_invader_data.ship_cannon_position_x,
                           space_invader_data.ship_cannon_position_y,
                           LASER,
                           LASER_0, LASER_0, 0, -space_invaders_level_data.laser_speed) )
    {
      sound_play_shoot();
      space_invader_data.last_shot_time = timer_count;
    }
  }
  else
  {
    GREEN_LED = 0x00;
  }

  // Enemies shooting logic
  if (space_invader_data.enemies_active_count > 0 && shoot_enable)
  {
    // The shooting probability of the enemies is determined
    // by the number of enemies remaing and the level
    unsigned int tempo = tempo_data[space_invader_data.enemies_active_count - 1].tempo;
    unsigned int shooting_p = tempo_data[space_invader_data.enemies_active_count - 1].shooting_p;
    shooting_p += space_invaders_level_data.shooting_probability_offset;

    // shooting probability is capped to avoid excesive shooting
    // in the higher levels
    if (shooting_p > SHOOTING_P_CAP) shooting_p = SHOOTING_P_CAP;

    // Play base tempo sounds
    space_invader_data.tempo_counter++;
    if (space_invader_data.tempo_counter > tempo)
    {
      space_invader_data.tempo_counter = 0;
      sound_play_fast_invader(space_invader_data.tempo_sequence++);
      space_invader_data.tempo_sequence &= 0x3;
    }

    // Determine if enemy should shoot.
    // shooting probability is calculated
    // as number of shots every 16 seconds
    if ( shooting_p > (Random32() % 480) )
    {
      enemy_random_shoot();
    }

    // Code to start UFO
    if ( (space_invaders_get_topmost_enemy_position() > (10 * 256)) &&
         !space_invader_data.ufo_passed)
    {
      enemy_init_sprite(-8, 7, SMALL_ENEMY_BONUS, SMALL_ENEMY_BONUS, ufo_tick, 100);
      space_invader_data.ufo_passed = true;
    }
  }
}

// Handle pushbutton and potentiometer
// Button inputs is converted to pulses
// to avoid continous shooting
void space_invaders_handle_inputs(void)
{
  if (PUSH_BUTTON_0)
  {
    if(!space_invader_data.button_a_previous_state)
    {
      if (!space_invader_data.special_mode)
        space_invader_data.button_a_previous_state = true;

      space_invader_data.button_a_pressed = true;
    }
    else
    {
      space_invader_data.button_a_pressed = false;
    }
  }
  else
  {
    space_invader_data.button_a_previous_state = false;
    space_invader_data.button_a_pressed = false;
  }

  if (PUSH_BUTTON_1)
  {
    if(!space_invader_data.button_b_previous_state)
    {
      space_invader_data.button_b_previous_state = true;
      space_invader_data.button_b_pressed = true;
    }
    else
    {
      space_invader_data.button_b_pressed = false;
    }
  }
  else
  {
    space_invader_data.button_b_previous_state = false;
    space_invader_data.button_b_pressed = false;
  }

  // Get ADC input
  space_invader_data.pot_position = adc_in();
}

// Draw lives icons on screen
void space_invaders_draw_lives(unsigned int lives)
{
  unsigned int pos_x = SCREENW - 8;
  size_t i;
  if (lives > MAX_LIVES_DISPLAY) lives = MAX_LIVES_DISPLAY;

  for (i = 1; i <= lives; i++)
  {
    graphical_draw_bitmap(display_buffer, SPACE_SHIP_LIFE, pos_x, 0, false);
    pos_x -= 8;
  }
}




// Collision Handler functions:
// These define the interaction
// between the sprites when they
// collide

// Enemy laser collision:
void enemy_laser(sprite_t* s1, sprite_t* s2)
{
  sprite_t* enemy;
  sprite_t* laser;

  enemy = s1->type == ENEMY ? s1 : s2;
  laser = s1->type == LASER ? s1 : s2;

  // Start explosion
  explosion_init_sprite(enemy->center_pos_x, enemy->center_pos_y,
                        SMALL_EXPLOSION_0, 0, 10);

  // enemy killed sound
  sound_play_enemy_killed();
  // Update score
  space_invader_data.score += ((enemy_data_t*)enemy->data)->value;
  // remove laser shot
  weapon_release_sprite(laser);
  // remove enemy
  enemy_release_sprite(enemy);
}

// Enemy ship collision:
void enemy_ship(sprite_t* s1, sprite_t* s2)
{
  sprite_t* ship;

  ship = s1->type == SHIP ? s1 : s2;

  explosion_init_sprite(ship->center_pos_x, ship->center_pos_y,
                        BIG_EXPLOSION_0, BIG_EXPLOSION_1, 10);

  sound_play_big_explosion();
  space_invaders_reset_enemy_position();
  
  // Special mode gives inmunity
  if (space_invader_data.special_mode) return;
  
  space_invader_data.cannon_alive = false;
  sprite_release(&sprites_vector, ship);
}

// Enemy bunker collision:
void enemy_bunker(sprite_t* s1, sprite_t* s2)
{
  sprite_t* bunker;

  bunker = s1->type == BUNKER ? s1 : s2;

  // Reduce bunker size by one pixel to avoid multiple collisions
  bunker->size_y_div_2 -= 256;
  if (bunker->size_y_div_2 < 0) bunker->size_y_div_2 = 0;
  bunker_degrade(bunker);
}

// missile laser collision handler:
void missile_laser(sprite_t* s1, sprite_t* s2)
{
  sprite_t* missile;
  sprite_t* laser;

  missile = s1->type == MISSILE ? s1 : s2;
  laser = s1->type == LASER ? s1 : s2;

  space_invader_data.score += 50;

  weapon_release_sprite(laser);
  weapon_release_sprite(missile);
}

// bunker laser collision handler:
void bunker_laser(sprite_t* s1, sprite_t* s2)
{
  sprite_t* bunker;
  sprite_t* laser;

  laser = s1->type == LASER ? s1 : s2;
  bunker = s1->type == BUNKER ? s1 : s2;

  weapon_release_sprite(laser);
  bunker_degrade(bunker);
}

// bunker missile collision handler:
void bunker_missile(sprite_t* s1, sprite_t* s2)
{
  sprite_t* bunker;
  sprite_t* missile;

  missile = s1->type == MISSILE ? s1 : s2;
  bunker = s1->type == BUNKER ? s1 : s2;

  weapon_release_sprite(missile);
  bunker_degrade(bunker);
}

// ship missile collision handler:
void ship_missile(sprite_t* s1, sprite_t* s2)
{
  sprite_t* ship;
  sprite_t* missile;

  missile = s1->type == MISSILE ? s1 : s2;
  ship = s1->type == SHIP ? s1 : s2;

  explosion_init_sprite(ship->center_pos_x, ship->center_pos_y,
                        BIG_EXPLOSION_0, BIG_EXPLOSION_1, 10);

  sound_play_big_explosion();
  weapon_release_sprite(missile);
  
  if (space_invader_data.special_mode) return;

  space_invader_data.cannon_alive = false;
  sprite_release(&sprites_vector, ship);
}


// Weapon related functions (Missiles and Lasers)
// These function handle the creation
// destruction and updates of weapons

// Get memory for a weapon specific data
weapon_data_t* weapon_data_activate(void)
{
  return (weapon_data_t*)vm_element_get(&weapon_data_vector);
}

// Release memory of a weapon specific data
void weapon_data_release(weapon_data_t* weapon_data)
{
  vm_element_release(&weapon_data_vector, (void*)weapon_data);
}


// Create a new Weapon sprite
sprite_t* weapon_init_sprite(int pos_x, int pos_y, int weapon_type,
                              const unsigned char* main_bitmap,
                              const unsigned char* alternate_bitmap,
                              int velocity_x, int velocity_y)
{
  // Get memory for the basic sprite data structure
  sprite_t* sprite = sprite_activate(&sprites_vector);
  weapon_data_t* data;

  if (!sprite) return 0;

  // Get memory for the weapons specific data
  data = weapon_data_activate();
  if (!data)
  {
    sprite_release(&sprites_vector, sprite);
    return 0;
  }

  memset(sprite, 0, sizeof(sprite_t));

  sprite->type = weapon_type;
  sprite->center_pos_x = pos_x;
  sprite->center_pos_y = pos_y;
  sprite->size_x_div_2 = main_bitmap[0] << 7;
  sprite->size_y_div_2 = main_bitmap[1] << 7;
  sprite->bitmap_ptr = main_bitmap;
  sprite->data = (void*)data;
  sprite->tick_function = weapon_tick;

  memset(data, 0, sizeof(weapon_data_t));
  data->alternate_bitmap = alternate_bitmap;
  data->bitmap_alternate_rate = 10;
  data->bitmap_alternate_rate_counter = 0;
  data->weapon_speed_x = velocity_x;
  data->weapon_speed_y = velocity_y;

  return sprite;
}

// Release the memory of a weapon sprite
void weapon_release_sprite(sprite_t* sprite)
{
  weapon_data_release((weapon_data_t*)sprite->data);
  sprite_release(&sprites_vector, sprite);
}

// Release all actives weapons
void weapon_release_all(void)
{
  size_t it = 0;

  // Iterate through all active sprites
  sprite_t* sprite = (sprite_t*)vm_iterate(&sprites_vector, &it);

  while (sprite)
  {
    // Check if sprite is Missile or Laser type (weapon)
    if (sprite->type == MISSILE || sprite->type == LASER)
    {
      weapon_release_sprite(sprite);
    }
    sprite = (sprite_t*)vm_iterate(&sprites_vector, &it);
  }
}

// This function is called from sprite_tick_all function
// It will be called for all active weapons on each
// refresh cycle (30ms)
void weapon_tick(sprite_t* sprite)
{
  weapon_data_t* data = (weapon_data_t*)sprite->data;
  if (data)
  {
    // Code to change the bitmap, used for the wiggle effect
    data->bitmap_alternate_rate_counter++;
    if (data->bitmap_alternate_rate_counter >= data->bitmap_alternate_rate)
    {
      const unsigned char* tmp = sprite->bitmap_ptr;
      sprite->bitmap_ptr = data->alternate_bitmap;
      data->alternate_bitmap = tmp;
      data->bitmap_alternate_rate_counter = 0;
    }

    // Update weapon position
    sprite->center_pos_x += data->weapon_speed_x;
    sprite->center_pos_y += data->weapon_speed_y;
  }

  // Check if weapon has gone out of the screen
  // and remove
  if (sprite->center_pos_x > (SCREENW << 8) ||
      sprite->center_pos_x < 0 ||
      sprite->center_pos_y > (SCREENH << 8) ||
      sprite->center_pos_y < 0)
  {
    weapon_release_sprite(sprite);
  }
}


// Explosion Related functions
explosion_data_t* explosion_data_activate(void)
{
  return (explosion_data_t*)vm_element_get(&explosion_data_vector);
}

void explosion_data_release(explosion_data_t* explosion_data)
{
  vm_element_release(&explosion_data_vector, explosion_data);
}

sprite_t* explosion_init_sprite(int pos_x, int pos_y,
                                const unsigned char* main_bitmap,
                                const unsigned char* alternate_bitmap,
                                unsigned int lifetime)
{
  sprite_t* sprite = sprite_activate(&sprites_vector);
  explosion_data_t* data;

  if (!sprite) return 0;

  data = explosion_data_activate();
  if (!data)
  {
    sprite_release(&sprites_vector, sprite);
    return 0;
  }

  memset(sprite, 0, sizeof(sprite_t));

  sprite->type = EXPLOSION;
  sprite->center_pos_x = pos_x;
  sprite->center_pos_y = pos_y;
  sprite->size_x_div_2 = main_bitmap[0] << 7;
  sprite->size_y_div_2 = main_bitmap[1] << 7;
  sprite->bitmap_ptr = main_bitmap;
  sprite->data = (void*)data;
  sprite->tick_function = explosion_tick;

  if (data)
  {
    memset(data, 0, sizeof(explosion_data_t));
    data->alternate_bitmap = alternate_bitmap;
    data->lifetime = lifetime;
  }
  return sprite;
}

void explosion_tick(sprite_t* sprite)
{
  explosion_data_t* data = (explosion_data_t*)sprite->data;
  if (data)
  {
    if (data->lifetime > 0) data->lifetime--;
    if (data->lifetime == 5 && data->alternate_bitmap)
      sprite->bitmap_ptr = data->alternate_bitmap;

    if (!data->lifetime) explosion_release_sprite(sprite);
  }
}

void explosion_release_sprite(sprite_t* sprite)
{
  explosion_data_release((explosion_data_t*)sprite->data);
  sprite_release(&sprites_vector, sprite);
}



// Enemy related functions
enemy_data_t* enemy_data_activate(void)
{
  return (enemy_data_t*)vm_element_get(&enemy_data_vector);
}

void enemy_data_release(enemy_data_t* data)
{
  vm_element_release(&enemy_data_vector, (void*)data);
}


sprite_t* enemy_init_sprite(int pos_x, int pos_y,
                            const unsigned char* main_bitmap,
                            const unsigned char* alternate_bitmap,
                            void(*tick_function)(sprite_t*), int value)
{
  sprite_t* sprite = sprite_activate(&sprites_vector);
  enemy_data_t* data = enemy_data_activate();
  if (!sprite) return 0;

  memset(sprite, 0, sizeof(sprite_t));

  sprite->type = ENEMY;
  // NOTE sprite representation of position and sizes
  // is pixel * 256 (equivalent to pixel << 8)
  sprite->center_pos_x = pos_x << 8;
  sprite->center_pos_y = pos_y << 8;
  // x << 7 is equivalent to x * 256 / 2;
  sprite->size_x_div_2 = main_bitmap[0] << 7;
  sprite->size_y_div_2 = main_bitmap[1] << 7;
  sprite->bitmap_ptr = main_bitmap;
  sprite->data = (void*)data;
  sprite->tick_function = tick_function;

  if (data)
  {
    memset(data, 0, sizeof(enemy_data_t));
    data->alternate_bitmap = alternate_bitmap;
    data->bitmap_alternate_rate = 10;
    data->bitmap_alternate_rate_counter = Random() % 9;
    data->value = value;
  }
  return sprite;
}

void enemy_release_sprite(sprite_t* sprite)
{
  int i;
  for (i = 0; i < MAX_ENEMIES; i++)
  {
    if (sprite == space_invader_data.enemies_active[i])
    {
      space_invader_data.enemies_active[i] = 0;
      space_invader_data.enemies_active_count--;
      break;
    }
  }

  enemy_data_release((enemy_data_t*)sprite->data);
  sprite_release(&sprites_vector, sprite);
}

// This function will determine which
// enemies are allowed to shoot (no other enemy in front)
// then chose one of the enemies and generate a Missile
bool enemy_random_shoot(void)
{
  sprite_t* enemies_in_shooting_position[ENEMY_COLUMNS];
  size_t index = 0;
  unsigned int i;
  // Check which enemies are clear to shoot
  for (i = 0; i < ENEMY_COLUMNS; i++)
  {
    if (space_invader_data.enemies_active[2 * ENEMY_COLUMNS + i])
      enemies_in_shooting_position[index++] = space_invader_data.enemies_active[2 * ENEMY_COLUMNS + i];
    else if (space_invader_data.enemies_active[ENEMY_COLUMNS + i])
      enemies_in_shooting_position[index++] = space_invader_data.enemies_active[ENEMY_COLUMNS + i];
    else if (space_invader_data.enemies_active[i])
      enemies_in_shooting_position[index++] = space_invader_data.enemies_active[i];
  }
  // Choose randomly one enemy to shoot
  if (index > 0)
  {
    index = Random() % index;
    if (weapon_init_sprite(enemies_in_shooting_position[index]->center_pos_x,
                           enemies_in_shooting_position[index]->center_pos_y,
                           MISSILE,
                           MISSILE_0, MISSILE_1, 0, space_invaders_level_data.missile_speed) )
    {
        return true;
    }
  }
   return false;
}


#define UFO_SPEED   (128)
void ufo_tick(sprite_t* sprite)
{
  sprite->center_pos_x += UFO_SPEED;
  if (((sprite->center_pos_x - sprite->size_x_div_2) >> 8) > SCREENW)
    enemy_release_sprite(sprite);
}


void enemy_tick(sprite_t* sprite)
{
  enemy_data_t* data = (enemy_data_t*)sprite->data;
  if (data)
  {
    data->bitmap_alternate_rate_counter++;
    if (data->bitmap_alternate_rate_counter >= data->bitmap_alternate_rate)
    {
      const unsigned char* tmp = sprite->bitmap_ptr;
      sprite->bitmap_ptr = data->alternate_bitmap;
      data->alternate_bitmap = tmp;
      data->bitmap_alternate_rate_counter = 0;
    }
  }
  sprite->center_pos_x += space_invader_data.enemy_speed_x;
  sprite->center_pos_y += space_invader_data.enemy_speed_y;

  if ( (sprite->center_pos_y >> 8) > SCREENH)
  {
    sprite->center_pos_y = 0;
  }
}

// Bunker related functions
sprite_t* bunker_init_sprite(int pos_x, int pos_y,
                             const unsigned char* bitmap)
{
  sprite_t* sprite = sprite_activate(&sprites_vector);
  if (!sprite) return 0;

  memset(sprite, 0, sizeof(sprite_t));

  sprite->type = BUNKER;
  sprite->center_pos_x = pos_x << 8;
  sprite->center_pos_y = pos_y << 8;
  sprite->size_x_div_2 = bitmap[0] << 7;
  sprite->size_y_div_2 = bitmap[1] << 7;
  sprite->bitmap_ptr = bitmap;
  sprite->data = 0;
  sprite->tick_function = 0;

  return sprite;
}

void bunker_degrade(sprite_t* sprite)
{
  // Dirty hack is used to store the bunker status
  // in the sprite->data void pointer by casting the pointer
  // as an unsigned int
  unsigned int bunker_status = (unsigned int)sprite->data;

  switch (bunker_status)
  {
    case 0:
      sprite->bitmap_ptr = BUNKER_1;
      sprite->data = (void*)1;
      break;
    case 1:
      sprite->bitmap_ptr = BUNKER_2;
      sprite->data = (void*)2;
      break;
    default:
      explosion_init_sprite(sprite->center_pos_x, sprite->center_pos_y,
                            SMALL_EXPLOSION_0, 0, 5);
      sprite_release(&sprites_vector, sprite);
  }
}

// Ship related functions
void ship_tick(sprite_t* sprite)
{
  // Lovely criptic code, up to you to work it out!
  sprite->center_pos_x = (space_invader_data.pot_position * ((SCREENW << 8) - (sprite->size_x_div_2 << 1 ))) >> 12;
  sprite->center_pos_x += sprite->size_x_div_2;

  space_invader_data.ship_cannon_position_x = sprite->center_pos_x;
  space_invader_data.ship_cannon_position_y = sprite->center_pos_y;
}


sprite_t* ship_init_sprite( const unsigned char* main_bitmap )
{
  sprite_t* sprite = sprite_activate(&sprites_vector);
  if (!sprite) return 0;

  memset(sprite, 0, sizeof(sprite_t));

  sprite->type = SHIP;
  sprite->center_pos_x = 0;
  sprite->center_pos_y = 44 << 8;
  sprite->size_x_div_2 = main_bitmap[0] << 7;
  sprite->size_y_div_2 = main_bitmap[1] << 7;
  sprite->bitmap_ptr = main_bitmap;
  sprite->data = 0;
  sprite->tick_function = ship_tick;

  return sprite;
}



