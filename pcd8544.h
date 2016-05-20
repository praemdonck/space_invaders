#ifndef _pcd8544_h
#define _pcd8544_h

#include <stdbool.h>
#include "ssi.h"

#define PIN_A3  (((volatile unsigned long *)(0x40004000 + (1 << (3+2)))))
#define PIN_A6  (((volatile unsigned long *)(0x40004000 + (1 << (6+2)))))
#define PIN_A7  (((volatile unsigned long *)(0x40004000 + (1 << (7+2)))))


typedef struct {
    ssi_ports_e ssi_port;
    volatile unsigned long* reset_pin;
    volatile unsigned long* data_command_pin;
    volatile unsigned long* chip_select_pin;
} pcd8544_data_t;

void pcd8544_init(pcd8544_data_t* display_data, 
                  ssi_ports_e ssi_port,
                  volatile unsigned long* reset_pin,
                  volatile unsigned long* data_command_pin,
                  volatile unsigned long* chip_select_pin);


void pcd8544_write_command(pcd8544_data_t* display_data, unsigned char command);
void pcd8544_set_function(pcd8544_data_t* display_data, bool pd, bool v, bool h);
void pcd8544_write_data(pcd8544_data_t* display_data, unsigned char* data, size_t len);
void pcd8544_set_display_control(pcd8544_data_t* display_data, bool d, bool e);
void pcd8544_set_y_address(pcd8544_data_t* display_data, unsigned char y_address);
void pcd8544_set_x_address(pcd8544_data_t* display_data, unsigned char x_address);
void pcd8544_set_temperature_control(pcd8544_data_t* display_data, unsigned char temp_control);
void pcd8544_set_bias_system(pcd8544_data_t* display_data, unsigned char bias);
void pcd8544_set_vop(pcd8544_data_t* display_data, unsigned char bias);
                  
#endif
