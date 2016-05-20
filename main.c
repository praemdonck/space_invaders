#include <string.h>
#include <stdio.h>
//#include "tm4c123gh6pm.h"
#include "ssi.h"
#include "pcd8544.h"
#include "graphical.h"
#include "hardware_driver.h"
#include "assets.h"
#include "sprites.h"
#include "vector_memory.h"
#include "space_invaders.h"
#include "breakout.h"
#include "Random.h"
#include "sound.h"

int main(void)
{ 
  unsigned int level = 0;

  clock_init();

  port_a_init();
  port_b_init();
  port_e_init();
  adc_init();

  ssi_init(SSI0, false);

  timer2_init(0x0028B0AA);

  Random_Init(10);
  sound_init();

  pcd8544_init(&display_data, SSI0, PIN_A7, PIN_A6, PIN_A3);

  space_invaders_set_sm(PUSH_BUTTON_0 && PUSH_BUTTON_1);
  space_invaders_show_splash(20);
  space_invaders_init();

  while (1)
  {
    char string[12];
    // Show next level
    // Note that levels shown to user start at 1
    // while internal level representation starts at 0
    graphical_clear_buffer(display_buffer);
    sprintf(string, "Level: %d", level + 1);
    graphical_draw_string(display_buffer, string, 10, 16, false);
    pcd8544_write_data(&display_data, display_buffer, DISPLAY_MEMORY_SIZE);
    delay_100_ms(15);

    space_invaders_set_level(level);

    // Start playing
    if (space_invaders_play_level())
    {
      // We are not dead yet, so move on
      // to next level
      level++;
      // check if we need to play breakout level
      if ((level % 5) == 0)
      {
        graphical_clear_buffer(display_buffer);
        sprintf(string, "Clear all");
        graphical_draw_string(display_buffer, string, 6, 0, false);
        sprintf(string, "bricks for");
        graphical_draw_string(display_buffer, string, 2, 16, false);
        sprintf(string, "extra life");
        graphical_draw_string(display_buffer, string, 2, 32, false);
        pcd8544_write_data(&display_data, display_buffer, DISPLAY_MEMORY_SIZE);

        delay_100_ms(30);

        // if we succesfully complete breakout
        // level, increment lives
        if (breakout_play_level())
        {
          space_invaders_increment_score(1);
        }
      }
    }
    else
    {
      // We are dead, restart from scratch
      graphical_clear_buffer(display_buffer);
      sprintf(string, "GAME OVER");
      graphical_draw_string(display_buffer, string, 6, 6, false);
      sprintf(string, "Score");
      graphical_draw_string(display_buffer, string, 22, 20, false);
      graphical_draw_number(display_buffer, space_invaders_get_score(), 32, 38);


      pcd8544_write_data(&display_data, display_buffer, DISPLAY_MEMORY_SIZE);
      delay_100_ms(50);

      space_invaders_show_splash(30);
      level = 0;
      space_invaders_init();
    }
  }
}
