#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "graphical.h"


#define SCREENW     84
#define SCREENH     48

// Graphical Routines

// Set a particular pixel in a buffer representing the display memory
// If x or y coordinates exceed the display size, don't do anything
void graphical_set_pixel(unsigned char* buffer, unsigned int x, unsigned int y)
{
  // Assume Horizontal memory alignment
  size_t byte_offset;
  char mask;

  if (x >= SCREENW || y >= SCREENH) return;

  byte_offset = (y / 8) * SCREENW + x;
  mask = 1 << (y % 8);
  buffer[byte_offset] |= mask;
}

// Clear a particular pixel in a buffer representing the display memory
// If x or y coordinates exceed the display size, don't do anything
void graphical_clear_pixel(unsigned char* buffer, unsigned int x, unsigned int y)
{
  // Assume Horizontal memory alignment
  size_t byte_offset;
  char mask;

  if (x >= SCREENW || y >= SCREENH) return;

  byte_offset = (y / 8) * SCREENW + x;
  mask = 1 << (y % 8);

  buffer[byte_offset] &= ~mask;
}

// Copy a bitmap image to a screen buffer
// x_pos and y_pos represent the position of the bitmap on the screen buffer. 
// centre_coordinate == false, x_pos and y_pos represent top-left corner position 
// of the bitmap on the screen
//
// centre_coordinate == true, x_pos and y_pos represent center position
// of the bitmap on the screen
void graphical_draw_bitmap(unsigned char* buffer, const unsigned char* bitmap, int x_pos, int y_pos, bool centre_coordinates)
{
  unsigned int size_x = bitmap[0];
  unsigned int size_y = bitmap[1];
  // size_y_bytes represent the number of row of bytes needed to represent the image
  unsigned int size_y_bytes = size_y / 8 + ((size_y % 8) ? 1 : 0);
  int x, y;

  // Adjust centre coordinates to top-left corner representation
  if (centre_coordinates)
  {
    x_pos -= size_x >> 1;
    y_pos -= size_y >> 1;
  }

  // Copy image bitmap to screen_buffer
  for (y = 0; y < size_y_bytes; y++)
  {
    unsigned int num_bits_in_row = (y + 1) * 8 <= size_y ? 8 : (size_y % 8);
    for (x = 0; x < size_x; x++)
    {
      bool value; 
      char current_byte = bitmap[2 + y * size_x + x];
      int temp_x = x + x_pos;
      int temp_y = (y * 8) + y_pos;
      int i;

      for (i = 0; i < num_bits_in_row; i++)
      {
        value = current_byte & 1;
        if (value && temp_x >= 0 && temp_y >= 0)
        {
          graphical_set_pixel(buffer, temp_x, temp_y);
        }
        temp_y++;
        current_byte >>= 1;
      }
    }
  }
}

const unsigned char NUMBERS[] = { 0x1F, 0x11, 0x1F,  /* 0 */
                                  0x00, 0x00, 0x1F,  /* 1 */
                                  0x1D, 0x15, 0x17,  /* 2 */
                                  0x15, 0x15, 0x1F,  /* 3 */
                                  0x07, 0x04, 0x1F,  /* 4 */
                                  0x17, 0x15, 0x1D,  /* 5 */
                                  0x1F, 0x14, 0x1C,  /* 6 */
                                  0x01, 0x01, 0x1F,  /* 7 */
                                  0x1F, 0x15, 0x1F,  /* 8 */
                                  0x07, 0x05, 0x1F   /* 9 */        
                                };

// Draw a five digit number on screen, if value is over 99999 it will rollover
void graphical_draw_number(unsigned char* buffer, unsigned int value, int x_pos, int y_pos)
{
  char num_string[6];
  unsigned char num_bitmap[22];
  size_t i;
  size_t next_pos = 2;

  // Rollover if above 99999
  if (value > 99999) value %= 100000;

  // convert int to string representation
  // including 0 padding
  sprintf(num_string, "%05d", value);
  
  for (i = 0; i < 5; i++)
  {
    // Convert the ASCII of the digits to a number
    // between 0-9 to use as an index
    size_t digit = num_string[i] - '0';
    
    // 1 is the only digit that is treated differently because
    // it is 1 pixel wide, all the other digits are 3 pixel wide
    if (digit == 1) 
    {
      // Copy the data for the digit in the temporary bitmap memory
      num_bitmap[next_pos++] = NUMBERS[1*3 + 2];
    }
    else
    {
      // Copy the data for the digit in the temporary bitmap memory
      memcpy(&num_bitmap[next_pos], &NUMBERS[digit * 3], 3);
      next_pos += 3;
    }
    // Add a blank pixel between digits
    num_bitmap[next_pos++] = 0;
  }
  // Calculate final size of the bitmap
  num_bitmap[0] = next_pos - 3;
  num_bitmap[1] = 5;
  // Draw the bitmap on the screen
  graphical_draw_bitmap(buffer, num_bitmap, x_pos, y_pos, false);
}


void graphical_clear_buffer(unsigned char* buffer)
{
  memset(buffer, 0x00, SCREENW * SCREENH / 8);
}

extern char font0[];
#define FONT_0_WIDTH     8
#define FONT_0_HEIGHT    16
void graphical_draw_character(unsigned char* buffer, unsigned char symbol, unsigned int x, unsigned int y, bool overwrite)
{
  unsigned int symbol_index;
  unsigned int temp, i, j;

  if (symbol > 127)
    return;

  symbol_index = symbol * FONT_0_HEIGHT;

  for (i = 0; i < FONT_0_HEIGHT; i++)
  {
    temp = font0[symbol_index + i];

    for (j = 0; j < FONT_0_WIDTH; j++)
    {
      if (temp & 0x01)
        graphical_set_pixel(buffer, x + j, y + i);
      else if (overwrite)
        graphical_clear_pixel(buffer, x + j, y + i);

      temp >>= 1;
    }
  }
}


void graphical_draw_string(unsigned char* buffer, char* string, unsigned int x, unsigned int y, bool overwrite)
{
  unsigned int temp_x = x;
  while (*string)
  {
    switch (*string)
    {
      case '\n':
        y += FONT_0_HEIGHT;
        break;
      case '\r':
        x = temp_x;
        break;
      default:
        graphical_draw_character(buffer, *string, x, y, overwrite);
        x += FONT_0_WIDTH;
    }
    string++;
  }
}

