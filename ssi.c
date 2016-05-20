#include "tm4c123gh6pm.h"
#include "ssi.h"

typedef struct
{
    unsigned long port_base;
    unsigned long ssi_base;
    unsigned long port_clock_enable_bit;
    unsigned long ssi_clock_enable_bit;
    unsigned char pmc_setting;
    unsigned char clk_pin;
    unsigned char fss_pin;
    unsigned char rx_pin;
    unsigned char tx_pin;
} ssi_pins_data_t;

#define AFSEL(index)  (*((volatile unsigned long*)(ssi_pins_data[index].port_base + 0x0420)))
#define PCTL(index)  (*((volatile unsigned long*)(ssi_pins_data[index].port_base + 0x052C)))
#define DEN(index)  (*((volatile unsigned long*)(ssi_pins_data[index].port_base + 0x051C)))
    
#define SSICR0(index)  (*((volatile unsigned long*)(ssi_pins_data[index].ssi_base)))
#define SSICR1(index)  (*((volatile unsigned long*)(ssi_pins_data[index].ssi_base + 0x04)))
#define SSIDR(index)  (*((volatile unsigned long*)(ssi_pins_data[index].ssi_base + 0x08)))
#define SSISR(index)  (*((volatile unsigned long*)(ssi_pins_data[index].ssi_base + 0x0C)))
#define SSICPSR(index)  (*((volatile unsigned long*)(ssi_pins_data[index].ssi_base + 0x10)))
#define SSICC(index)  (*((volatile unsigned long*)(ssi_pins_data[index].ssi_base + 0x0FC8)))    
    
static const ssi_pins_data_t ssi_pins_data[] = {
    //Port base  SSI base     ce   sce   pmc  clk  fss   rx  tx
    {0x40004000, 0x40008000, 0x01, 0x01,   2,   2,   3,   4,  5},  // SSI0 Port A
    {0x40025000, 0x40009000, 0x20, 0x02,   2,   2,   3,   0,  1},  // SSI1 Port F
    {0x40007000, 0x40009000, 0x08, 0x02,   2,   0,   1,   2,  3},  // SSI1_ALT Port D
    {0x40005000, 0x4000A000, 0x02, 0x04,   2,   4,   5,   6,  7},  // SSI2 Port B
    {0x40007000, 0x4000B000, 0x08, 0x08,   1,   0,   1,   2,  3},  // SSI3 Port D
};



void ssi_init(ssi_ports_e ssi_index, bool config_fss_pin)
{
    if (ssi_index >= SSI_MAX)
        return;      
     
    SYSCTL_RCGCGPIO_R |= ssi_pins_data[ssi_index].port_clock_enable_bit;
    SYSCTL_RCGCSSI_R |= ssi_pins_data[ssi_index].ssi_clock_enable_bit;
    
    AFSEL(ssi_index) |= 1 << ssi_pins_data[ssi_index].clk_pin | 
                        1 << ssi_pins_data[ssi_index].rx_pin | 
                        1 << ssi_pins_data[ssi_index].tx_pin;
    
    DEN(ssi_index)   |= 1 << ssi_pins_data[ssi_index].clk_pin | 
                        1 << ssi_pins_data[ssi_index].rx_pin | 
                        1 << ssi_pins_data[ssi_index].tx_pin;

    PCTL(ssi_index) &= ~(0xF << ssi_pins_data[ssi_index].clk_pin * 4) &
                       ~(0xF << ssi_pins_data[ssi_index].fss_pin * 4) &     
                       ~(0xF << ssi_pins_data[ssi_index].rx_pin * 4)  & 
                       ~(0xF << ssi_pins_data[ssi_index].tx_pin * 4);

    
    PCTL(ssi_index) |= ssi_pins_data[ssi_index].pmc_setting << ssi_pins_data[ssi_index].clk_pin * 4 |
                       ssi_pins_data[ssi_index].pmc_setting << ssi_pins_data[ssi_index].rx_pin * 4  | 
                       ssi_pins_data[ssi_index].pmc_setting << ssi_pins_data[ssi_index].tx_pin * 4;
    
    if (config_fss_pin)
    {
        AFSEL(ssi_index) |= 1 << ssi_pins_data[ssi_index].fss_pin;
        DEN(ssi_index) |= 1 << ssi_pins_data[ssi_index].fss_pin;
        PCTL(ssi_index) |= ssi_pins_data[ssi_index].pmc_setting << ssi_pins_data[ssi_index].fss_pin * 4;
    }
    else
    {
        AFSEL(ssi_index) &= ~(1 << ssi_pins_data[ssi_index].fss_pin);
    }
          
    // Disable SSI
    SSICR1(ssi_index) &= ~SSI_CR1_SSE;
    
    
    // SCR = 0; SPH = 0; SPO = 0; FRF = 0 (Freescale SPI); DSS = 7 (8 bits) 
    SSICR0(ssi_index) = SSI_CR0_DSS_8;
    SSICC(ssi_index) = SSI_CC_CS_SYSPLL;
    SSICPSR(ssi_index) = 80;
    
    // Enable SSI in master mode
    SSICR1(ssi_index) = SSI_CR1_SSE;// | SSI_CR1_LBM;
}

void ssi_write_buffer(ssi_ports_e ssi_index, char* data, size_t length)
{
    while (length--)
    {
        while ( !(SSISR(ssi_index) & SSI_SR_TNF) ) {};
        SSIDR(ssi_index) = *data++;
    }
  
    while ( (SSISR(ssi_index) & SSI_SR_BSY) ) {};
}
    
    
/*
void ssi_write_byte(ssi_ports_e ssi_index, char data)
{
    int temp, temp1;
    data = 0;
    
    for (data = 0; data < 8; data++) SSIDR(ssi_index) = data;
  
    //while ( !(SSISR(ssi_index) & SSI_SR_TFE) );
    while (!(SSISR(ssi_index) & SSI_SR_RFF) ){};
    
}
*/

/*
void ssi_read_byte(ssi_ports_e ssi_index)
{
    int temp[20];
    int data = 0;
    while ( SSISR(ssi_index) & SSI_SR_RNE )
    {
        temp[data] = SSIDR(ssi_index);
        data++;
    }
    temp[data] = data;
}
*/

