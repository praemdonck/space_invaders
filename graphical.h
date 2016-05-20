#ifndef _graphical_h
#define _graphical_h

#include <stdbool.h>

// Graphical Functions
void graphical_set_pixel(unsigned char* buffer, unsigned int x, unsigned int y);
void graphical_clear_pixel(unsigned char* buffer, unsigned int x, unsigned int y);
void graphical_clear_buffer(unsigned char* buffer);
void graphical_draw_bitmap(unsigned char* buffer, const unsigned char* bitmap, int x_pos, int y_pos, bool centre_coordinates);
void graphical_draw_number(unsigned char* buffer, unsigned int value, int x_pos, int y_pos);
void graphical_draw_string(unsigned char* buffer, char* string, unsigned int x, unsigned int y, bool overwrite);
void graphical_draw_character(unsigned char* buffer, unsigned char symbol, unsigned int x, unsigned int y, bool overwrite);


#endif

