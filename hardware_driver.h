#ifndef _hardware_driver
#define _hardware_driver

#include "pcd8544.h"

#define SCREENW     84
#define SCREENH     48
#define DISPLAY_MEMORY_SIZE  (SCREENW * SCREENH / 8)

#define GREEN_LED  (*((volatile unsigned long *)0x420A7F90))
#define RED_LED    (*((volatile unsigned long *)0x420A7F94))


#define PUSH_BUTTON_0     (*((volatile unsigned long *)0x42487F80))
#define PUSH_BUTTON_1     (*((volatile unsigned long *)0x42487F84))

void clock_init(void);
void port_a_init(void);
void port_b_init(void);
void port_e_init(void);
void adc_init(void);
unsigned long adc_in(void);
void delay_100_ms(unsigned long count);

void timer2_init(unsigned long period);

// Global variables shared with timer interrupt routine
extern volatile unsigned long timer_count;
extern volatile unsigned long timer_semaphore;
extern unsigned char display_buffer[];
extern pcd8544_data_t display_data;
#endif
