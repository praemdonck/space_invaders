Space Invaders

Implementation of Space Invaders Game based on Texas Instrument 
Launchpad "Tiva C Series TM4C123G" - EK-TM4C123GXL and a 84x48
monochrome display with PCD8544 controller known as Nokia 5110 display.

This project started as Lab 15 for "Embedded Systems - Shape the World" 
EDX course given by the University of Texas

https://courses.edx.org/courses/course-v1:UTAustinX+UT.6.03x+1T2016/info

The original version of the game was modified to remove most of the 
libraries provided in the course and to re architect the code in modules.

The project should open and compile with uKeil 4 tool chain

Hardware Connections:

 ******* Required Hardware I/O connections*******************
 Slide pot pin 1 connected to ground
 Slide pot pin 2 connected to PE2/AIN1
 Slide pot pin 3 connected to +3.3V 
 fire button connected to PE0
 special weapon fire button connected to PE1
 8*R resistor DAC bit 0 on PB0 (least significant bit)
 4*R resistor DAC bit 1 on PB1
 2*R resistor DAC bit 2 on PB2
 1*R resistor DAC bit 3 on PB3 (most significant bit)
 LED on PB4
 LED on PB5

 Blue Nokia 5110
 ---------------
 Signal        (Nokia 5110) LaunchPad pin
 Reset         (RST, pin 1) connected to PA7
 SSI0Fss       (CE,  pin 2) connected to PA3
 Data/Command  (DC,  pin 3) connected to PA6
 SSI0Tx        (Din, pin 4) connected to PA5
 SSI0Clk       (Clk, pin 5) connected to PA2
 3.3V          (Vcc, pin 6) power
 back light    (BL,  pin 7) not connected, consists of 4 white LEDs which draw ~80mA total
 Ground        (Gnd, pin 8) ground

 Red SparkFun Nokia 5110 (LCD-10168)
 -----------------------------------
 Signal        (Nokia 5110) LaunchPad pin
 3.3V          (VCC, pin 1) power
 Ground        (GND, pin 2) ground
 SSI0Fss       (SCE, pin 3) connected to PA3
 Reset         (RST, pin 4) connected to PA7
 Data/Command  (D/C, pin 5) connected to PA6
 SSI0Tx        (DN,  pin 6) connected to PA5
 SSI0Clk       (SCLK, pin 7) connected to PA2
 back light    (LED, pin 8) not connected, consists of 4 white LEDs which draw ~80mA total



