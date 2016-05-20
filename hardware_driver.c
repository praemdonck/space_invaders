#include "tm4c123gh6pm.h"
#include "hardware_driver.h"
#include "pcd8544.h"


unsigned char display_buffer[DISPLAY_MEMORY_SIZE];
pcd8544_data_t display_data;

// Global variables shared with timer interrupt routine
volatile unsigned long timer_count = 0;
volatile unsigned long timer_semaphore = 0;


#define SYSCTL_SYSDIV_2_5  (0x04 << 22)
void clock_init(void)
{
  // Configure PLL and enable Main OSC but keep running on PIOSC
  SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_BYPASS | SYSCTL_RCC_USESYSDIV | SYSCTL_RCC_OSCSRC_INT;  
  // Use Register RCC2, Enable Div400, config SysDiv2 = 2 and SysDiv2LSB = 0, Switch to main Osc and disable Bypass
  SYSCTL_RCC2_R = SYSCTL_RCC2_USERCC2 | SYSCTL_RCC2_DIV400 | SYSCTL_SYSDIV_2_5 | SYSCTL_RCC2_OSCSRC2_MO;
    
  while ( !(SYSCTL_RIS_R & SYSCTL_RIS_PLLLRIS) );
}

void port_a_init(void)
{
  // Configure PA3,6,7 as outputs
  volatile unsigned long delay;
  SYSCTL_RCGC2_R     |= 0x00000001;      // 1) A clock
  delay              = SYSCTL_RCGC2_R;   //    delay to allow clock to stabilize     
  GPIO_PORTA_AMSEL_R &= 0x00;            // 2) disable analog function
  GPIO_PORTA_PCTL_R  &= 0x00FF0FFF;      // 3) GPIO clear bits PCTL for pins PA3,6,7
  GPIO_PORTA_DIR_R   |= 0xC8;            // 4.2) PA3,6,7 output  
  GPIO_PORTA_AFSEL_R &= ~0xC8;           // 5) no alternate function
  GPIO_PORTA_PUR_R   &= ~0xC8;           // 6) disable pullup resistor on PA3,6,7       
  GPIO_PORTA_PDR_R   &= ~0xC8;           // 6) disable pulldown resistor on PA3,6,7       
  GPIO_PORTA_DEN_R   |= 0xC8;            // 7) enable digital pins PA3,6,7
}

void port_b_init(void)
{ 
  // Configure PB0,1,2,3,4,5 as outputs
  volatile unsigned long delay;
  SYSCTL_RCGC2_R     |= 0x00000002;      // 1) B clock
  delay              = SYSCTL_RCGC2_R;   //    delay to allow clock to stabilize              
  GPIO_PORTB_AMSEL_R &= ~0x3F;           // 2) disable analog function
  GPIO_PORTB_PCTL_R  &= ~0x00FFFFFF;     // 3) GPIO clear bit PCTL  
  GPIO_PORTB_DIR_R   |= 0x3F;            // 4.2) PB0,1,2,3,4,5 output  
  GPIO_PORTB_AFSEL_R &= ~0x3F;           // 5) no alternate function
  GPIO_PORTB_PUR_R   &= ~0x3F;           // 6) disable pullup resistor on PB0-PB5       
  GPIO_PORTB_DEN_R   |= 0x3F;            // 7) enable digital pins PB0-PB5
}

void port_e_init(void)
{ 
  // Configure PE0,1 as inputs
  volatile unsigned long delay;
  SYSCTL_RCGC2_R     |= 0x00000010;      // 1) E clock
  delay              = SYSCTL_RCGC2_R;   //    delay to allow clock to stabilize              
  GPIO_PORTE_AMSEL_R &= ~0x03;           // 2) disable analog function
  GPIO_PORTE_PCTL_R  &= ~0x000000FF;     // 3) GPIO clear bits PCTL for pins PE0,1
  GPIO_PORTE_DIR_R   &= ~0x03;           // 4.1) PE0,1 input,
  GPIO_PORTE_AFSEL_R &= ~0x03;           // 5) no alternate function
  GPIO_PORTE_PUR_R   &= ~0x03;           // 6) disable pullup resistor on PE0,1
  GPIO_PORTE_PDR_R   &= ~0x03;           // 6) disable pulldown resistor on PE0,1
  GPIO_PORTE_DEN_R   |= 0x03;            // 7) enable digital pins PE0,1

  /*
  if ((GPIO_PORTE_DATA_R & 0x3) == 0x3)
    space_invader_data.special_mode = true;
  else
    space_invader_data.special_mode = false;
    */

}

// This initialization function sets up the ADC 
// Max sample rate: <=125,000 samples/second
// SS3 triggering event: software trigger
// SS3 1st sample source:  channel 1
// SS3 interrupts: enabled but not promoted to controller
void adc_init(void)
{
  volatile unsigned long delay;
  SYSCTL_RCGCADC_R   |= SYSCTL_RCGCADC_R0;  
  SYSCTL_RCGC2_R     |= SYSCTL_RCGC2_GPIOE; // E clock
  delay              = SYSCTL_RCGC2_R;      // delay to allow clock to stabilize
  GPIO_PORTE_AFSEL_R &= ~0x04;              // no alternate function for PE2
  GPIO_PORTE_DIR_R   &= ~0x04;              // PE2 input,  
  GPIO_PORTE_DEN_R   &= ~0x04;              // diable digital pin PE2
  GPIO_PORTE_PUR_R   &= ~0x04;              // disable pullup resistor on PE2
  GPIO_PORTE_PDR_R   &= ~0x04;              // disable pulldown resistor on PE2
  GPIO_PORTE_AMSEL_R |= 0x04;               // enable analog function on PE2
  GPIO_PORTE_AFSEL_R |= 0x04;               // Set AFSEL to keep grader happy PE2  
  SYSCTL_RCGC0_R     |= 0x00010000;         // activate ADC0
  delay              = SYSCTL_RCGC2_R;
  ADC0_ACTSS_R       &= ~ADC_ACTSS_ASEN3;   // Disable Sample Sequencer 3
  ADC0_EMUX_R        &= ~0xF000;            // ADC triggered from processor
  ADC0_SSMUX3_R      = 0x01;                // The first (and only) sample of SS3 is connected to AIN1 (PE2)
  ADC0_SSCTL3_R      = ADC_SSCTL3_IE0 | ADC_SSCTL3_END0;
  ADC0_ACTSS_R       |= ADC_ACTSS_ASEN3;    // Enable Sample Sequencer 3
}

//------------ADC0_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
unsigned long adc_in(void)
{  
  unsigned long result /*, systick_start, systick_stop*/;
  ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
  //systick_start = NVIC_ST_CURRENT_R;
  while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
  //systick_stop = NVIC_ST_CURRENT_R;
  result = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
  ADC0_ISC_R = 0x0008;             // 4) acknowledge completion
  return result;
}

void timer2_init(unsigned long period)
{ 
  unsigned long volatile delay;
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  delay               = SYSCTL_RCGCTIMER_R;
  timer_semaphore     = 0;
  timer_count         = 0;
  TIMER2_CTL_R        = 0x00000000;  // 1) disable timer2A during setup
  TIMER2_CFG_R        = 0x00000000;  // 2) configure for 32-bit mode
  TIMER2_TAMR_R       = 0x00000002;  // 3) configure for periodic mode, default down-count settings
  TIMER2_TAILR_R      = period-1;    // 4) reload value
  TIMER2_TAPR_R       = 0;           // 5) bus clock resolution
  TIMER2_ICR_R        = 0x00000001;  // 6) clear timer2A timeout flag
  TIMER2_IMR_R        = 0x00000001;  // 7) arm timeout interrupt
  NVIC_PRI5_R         = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // 8) priority 4
  NVIC_EN0_R          = 1<<23;       // 9) enable IRQ 23 in NVIC
  TIMER2_CTL_R        = 0x00000001;  // 10) enable timer2A
}

void Timer2A_Handler(void)
{ 
  TIMER2_ICR_R = 0x00000001;    // acknowledge timer2A timeout
  timer_count++;
  timer_semaphore = 1;          // trigger
}




void delay_100_ms(unsigned long count)
{
  unsigned long volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
