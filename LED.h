#ifndef __SDLIB_H
#define __SDLIB_H

  #include <msp430.h>  

  void SPI_LED_setup (void); // using UCB0 SPI modual P2.5,6,7 --> SOMI,SIMO,CLK
  void write_LED (int);      // will send data to the LED driver

  //These are the pins that will talk to the shift register through SPI <-- Port 2
  #define LE    BIT3  // Latch data to the driver passive low --> active high
  #define OE    BIT4  // Output enable, internal pull ups --> active low
  #define SDO   BIT5  // Data out (SOMI)
  #define SDI   BIT6  // Data in (SIMO)
  #define CLK   BIT7  // Clock

#endif
