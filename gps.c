#include <msp430f6779a.h>
#include "gps.h"

char gps[100];

void gps_setup(void){
  gps[0] = 0x21; // "!" to indicate an unaltered register

  //P1DIR |= BIT2; // Pin 1.2 is output
  //P1SEL0 |= BIT2; // Pin 1.2 output ACLK
  P3SEL0 |= BIT0 + BIT1; // UCA0RX + UCA0TXD

  UCA0CTLW0 = UCSWRST; // open UCA1 control registers
  //ACLK chosen, UCA0 needs 32kHz signal to communicate with GPS
  UCA0CTLW0 |= UCSSEL__ACLK; // use ACLK
  // N = BRCLK / baud rate = 32768 / 4800 = 6.8266666666666666666666666666667
  // eUCBR = INT(N) = INT(6.8266666666666666666666666666667) = 6
  //UCA1BR0 = 6; UCA1BR1 = 0; // UCBRx = UCAxBR0 + (UCAxBR1 * 256)
  UCA0BRW = 6;
  // eUCBRS = N - INT(N) = 6.827 - 6 = 0.827 -> UCBRS = 0xFF
  UCA0MCTLW = 0xEE00; // Oversampling disabled, UCBRS

  //Re-enable the module, and allow interrupts
  UCA0CTLW0 &= ~UCSWRST;
  UCA0IE |= UCRXIE;

//  //Caleb Alkire 4/4/17 GPS uart communication setup
//  //Reset values, and enable register editing
//  P3SEL0 |= BIT0 + BIT1;  // UCA0RX + UCA0TXD
//  UCA0CTL1 = UCSWRST;
//  //ACLK chosen, UCA0 needs 32kHz signal to communicate with GPS
//  UCA0CTL1 |= UCSSEL__ACLK;
//
//  //Set baud rate to 4800
//  UCA0BR1 |= 6;
//  UCA0MCTLW |= UCBRF_0 + UCBRS7;
//
//  //Re-enable the module, and allow interrupts
//  UCA0CTL1 &= ~UCSWRST;
//  UCA0IE |=  UCRXIE;
}


float gps_mintodeg(float tempmin)
{
  return tempmin / 60;
}


float gps_sectodeg(float tempsec)
{
  return tempsec / 3600;
}
