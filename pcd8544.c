#include "pcd8544.h"

#define DATA      0xFF
#define COMMAND   0x00
#define PIN_SET   0xFF
#define PIN_CLEAR 0x00


#define FUNCTION_SET_COMMAND     0x20
#define DISPLAY_CONTROL_COMMAND  0x08
#define SET_Y_ADDRESS_COMMAND    0x40
#define SET_X_ADDRESS_COMMAND    0x80
#define SET_TEMP_CONTROL_COMMAND 0x04
#define SET_BIAS_COMMAND         0x10
#define SET_VOP_COMMAND          0x80

void pcd8544_init(pcd8544_data_t* display_data, 
                  ssi_ports_e ssi_port,
                  volatile unsigned long* reset_pin,
                  volatile unsigned long* data_command_pin,
                  volatile unsigned long* chip_select_pin)
{
  display_data->ssi_port = ssi_port;
  display_data->reset_pin = reset_pin;
  display_data->data_command_pin = data_command_pin;
  display_data->chip_select_pin = chip_select_pin;

  *display_data->chip_select_pin = PIN_SET;
  *display_data->data_command_pin = COMMAND;
  *display_data->reset_pin = PIN_SET;

  // Set PD = 0 -> chip Active, 
  //     V  = 0 -> Horizontal memory alignment, 
  //     H  = 1 -> Extended instruction Set 
  pcd8544_set_function(display_data, false, false, true);
  pcd8544_set_vop(display_data, 0x40);
  pcd8544_set_temperature_control(display_data, 0);
  pcd8544_set_bias_system(display_data, 3);

  // Set PD = 0 -> chip Active, 
  //     V  = 0 -> Horizontal memory alignment, 
  //     H  = 1 -> Basic instruction Set 
  pcd8544_set_function(display_data, false, false, false);

  // Normal mode
  pcd8544_set_display_control(display_data, true, false);

}

void pcd8544_write_data(pcd8544_data_t* display_data, unsigned char* data, size_t len)
{
  *display_data->chip_select_pin = PIN_CLEAR;
  *display_data->data_command_pin = DATA;

  ssi_write_buffer(display_data->ssi_port, (char*)data, len);

  *display_data->chip_select_pin = PIN_SET;
}

void pcd8544_write_command(pcd8544_data_t* display_data, unsigned char command)
{
  *display_data->chip_select_pin = PIN_CLEAR;
  *display_data->data_command_pin = COMMAND;

  ssi_write_buffer(display_data->ssi_port, (char*)&command, 1);

  *display_data->chip_select_pin = PIN_SET;
}

void pcd8544_set_function(pcd8544_data_t* display_data, bool pd, bool v, bool h)
{
  unsigned char command = FUNCTION_SET_COMMAND;
  command |= pd ? 0x4 : 0x0; 
  command |= v  ? 0x2 : 0x0; 
  command |= h  ? 0x1 : 0x0; 
  pcd8544_write_command(display_data, command);
}

void pcd8544_set_display_control(pcd8544_data_t* display_data, bool d, bool e)
{
  unsigned char command = DISPLAY_CONTROL_COMMAND;
  command |= d ? 0x4 : 0x0; 
  command |= e ? 0x1 : 0x0; 
  pcd8544_write_command(display_data, command);
}

void pcd8544_set_y_address(pcd8544_data_t* display_data, unsigned char y_address)
{
  unsigned char command = SET_Y_ADDRESS_COMMAND;
  if (y_address > 5) y_address = 5;
  command |= y_address; 
  pcd8544_write_command(display_data, command);
}

void pcd8544_set_x_address(pcd8544_data_t* display_data, unsigned char x_address)
{
  unsigned char command = SET_X_ADDRESS_COMMAND;
  if (x_address > 83) x_address = 83;
  command |= x_address; 
  pcd8544_write_command(display_data, command);
}

void pcd8544_set_temperature_control(pcd8544_data_t* display_data, unsigned char temp_control)
{
  unsigned char command = SET_TEMP_CONTROL_COMMAND;
  if (temp_control > 3) temp_control = 3;
  command |= temp_control; 
  pcd8544_write_command(display_data, command);
}

void pcd8544_set_bias_system(pcd8544_data_t* display_data, unsigned char bias)
{
  unsigned char command = SET_BIAS_COMMAND;
  if (bias > 7) bias = 7;
  command |= bias; 
  pcd8544_write_command(display_data, command);
}

void pcd8544_set_vop(pcd8544_data_t* display_data, unsigned char vop)
{
  unsigned char command = SET_VOP_COMMAND;
  if (vop > 127) vop = 127;
  command |= vop; 
  pcd8544_write_command(display_data, command);
}

