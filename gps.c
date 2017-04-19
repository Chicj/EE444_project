#include <msp430f6779a.h>
#include "gps.h"

void gps_setup(void){
  //Caleb Alkire 4/4/17 GPS uart communication setup
  //Reset values, and enable register editing
  P1SEL0 |= BIT2;
  P3SEL0 |= BIT0 + BIT1;  // UCA0RX + UCA0TXD
  UCA0CTL1 = UCSWRST;
  //ACLK chosen, UCA0 needs 32kHz signal to communicate with GPS
  UCA0CTL1 |= UCSSEL__ACLK;

  //Set baud rate to 406800
  UCA0BR0 |= 6;
  UCA0BR1 |= 0;
  UCA0MCTLW |= UCBRF_0 + UCBRS7;

  //Re-enable the module, and allow interrupts
  UCA0CTL1 &= ~UCSWRST;
  UCA0IE |=  UCRXIE;
}


float gps_mintodeg(float tempmin)
{
  return tempmin / 60;
}


float gps_sectodeg(float tempsec)
{
  return tempsec / 3600;
}
