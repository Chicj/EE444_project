#include <msp430f6779a.h>
#include "LED.h"

void SPI_LED_setup(void){ // using UCB0 SPI modual P2.5,6,7 --> SOMI,SIMO,CLK
  P2SEL0 |= SDI + SDO + CLK; // select SPI functionality 
  P2DIR |= SDI + CLK + LE + OE; // set outputs
  P2OUT &= ~LE; // Enable output drivers (active low), LE must start low (passive low anyway )

/* SPI set up */
  UCB0CTLW0 |= UCSWRST;
  UCB0CTLW0 =UCMST|UCCKPL|UCMSB|UCSYNC|UCSSEL_2|UCSWRST;     // 3-pin, 8-bit SPI master, SMCLK
  UCB0BRW = 100;    
  UCB0CTLW0 &=~UCSWRST;                      //Initialize state machine
  
}

void write_LED(int input){  // will send out 
  char i = 0;

  char tx_buf[2],rx_buf[2];

  P2OUT |= LE;  // set latch high to write data
  tx_buf[1]=input>>8;
  tx_buf[0]=input;

  while(i < 2){ // push 2 bytes to the LED driver
    UCB0TXBUF = tx_buf[i];  // stick a byte in the buffer
    while(!(UCB0IFG && UCTXIFG)){  // wait for TX buffer
    }
    rx_buf[i] = UCB0RXBUF ; // check good write (dont need this for anything really)
    while(!(UCB0IFG && UCRXIFG)){  // wait for RX buffer
    }
    i++;  // count transmission looking for 2 bytes
  }
  P2OUT &= ~LE + ~OE;  // Set latch low to latch data and enable output
}
