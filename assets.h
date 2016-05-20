#ifndef _assets_h
#define _assets_h

// enemy ship that starts at the top of the screen (arms/mouth closed)
// width=18 x height=8
extern const unsigned char SMALL_ENEMY_30_A[];

// enemy ship that starts at the top of the screen (arms/mouth open)
// width=10 x height=8
extern const unsigned char SMALL_ENEMY_30_B[];

// enemy ship that starts in the middle of the screen (arms together)
// width=12 x height=8
extern const unsigned char SMALL_ENEMY_20_A[];

// enemy ship that starts in the middle of the screen (arms apart)
// width=14 x height=8
extern const unsigned char SMALL_ENEMY_20_B[];

// enemy ship that starts at the bottom of the screen (arms down)
// width=12 x height=8
extern const unsigned char SMALL_ENEMY_10_A[];

// enemy ship that starts at the bottom of the screen (arms up)
// width=12 x height=8
extern const unsigned char SMALL_ENEMY_10_B[];

// image of the player's ship
// includes two blacked out columns on the left and right sides of the image to prevent smearing when moved 2 pixels to the left or right
// width=14 x height=7
extern const unsigned char PLAYER_SHIP[];

// small, fast bonus enemy that occasionally speeds across the top of the screen after enough enemies have been killed to make room for it
// includes two blacked out columns on the left and right sides of the image to prevent smearing when moved 2 pixels to the left or right
// width=16 x height=7
extern const unsigned char SMALL_ENEMY_BONUS[];

// small shield floating in space to cover the player's ship from enemy fire (undamaged)
// width=18 x height=5
extern const unsigned char BUNKER_0[];

// small shield floating in space to cover the player's ship from enemy fire (moderate generic damage)
// width=18 x height=5
extern const unsigned char BUNKER_1[];

// small shield floating in space to cover the player's ship from enemy fire (heavy generic damage)
// width=18 x height=5
extern const unsigned char BUNKER_2[];

// large explosion that can be used upon the demise of the player's ship (first frame)
// width=18 x height=8
extern const unsigned char BIG_EXPLOSION_0[];
    
// large explosion that can be used upon the demise of the player's ship (second frame)
// width=18 x height=8
extern const unsigned char BIG_EXPLOSION_1[];
    
// small explosion best used for the demise of an enemy
// width=16 x height=10
extern const unsigned char SMALL_EXPLOSION_0[];

// a missile in flight
// includes one blacked out row on the top, bottom, and right of the image to prevent smearing when moved 1 pixel down, up, or left
// width=3 x height=7
extern const unsigned char MISSILE_0[];

// a missile in flight
// includes one blacked out row on the top, bottom, and left of the image to prevent smearing when moved 1 pixel down, up, or right
// width=3 x height=7
extern const unsigned char MISSILE_1[];

// a laser burst in flight
// includes one blacked out row on the top and bottom of the image to prevent smearing when moved 1 pixel up or down
// width=2 x height=7
extern const unsigned char LASER_0[];

// Space invaders splash screen
extern const unsigned char SPACE_INVADER_SPLASH[];

// Remaining Lifes indicator
extern const unsigned char SPACE_SHIP_LIFE[];


extern const unsigned char BREAKOUT_BRICK[];
extern const unsigned char BREAKOUT_PADDLE[];
extern const unsigned char BREAKOUT_BALL[];


#endif

