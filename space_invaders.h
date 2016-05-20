#ifndef _space_invader_h
#define _space_invader_h

#include <stdbool.h>

#define MAX_ENEMIES    (15)
#define MAX_BUNKERS    (2)
#define MAX_SHIPS      (1)
#define MAX_WEAPONS    (60)
#define MAX_EXPLOSIONS (4)

#define MAX_SPRITES  (82)



#define ENEMY_COLUMNS  5 

//Num lives
#define LIVES (3)
// Enemy Missiles base speed
#define MISSILE_SPEED 64
// Canon laser speed
#define LASER_SPEED   256

// Enemies max speed
#define MAX_SPEED_CAP  (5*256)

// Enemies max shooting probablity
#define SHOOTING_P_CAP  (12)

// Max lives to display to avoid overwriting the score
#define MAX_LIVES_DISPLAY 7


void space_invaders_set_level(unsigned int level);
unsigned int space_invaders_play_level(void);
void space_invaders_show_splash(unsigned int delay);
void space_invaders_init(void);
int space_invaders_get_score(void);
void space_invaders_increment_score(int n);
void space_invaders_set_sm(bool val);

#endif
