#ifndef _SSI_H
#define _SSI_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
    SSI0,
    SSI1,
    SSI1_ALT,
    SSI2,
    SSI3,
    SSI_MAX
} ssi_ports_e;

void ssi_init(ssi_ports_e ssi_index, bool config_fss_pin);
void ssi_write_byte(ssi_ports_e ssi_index, char data);
void ssi_write_buffer(ssi_ports_e ssi_index, char* data, size_t length);
void ssi_read_byte(ssi_ports_e ssi_index);


#endif
