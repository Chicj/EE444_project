#include <msp430f6779a.h>
#include "gps.h"
#include "pathfinding.h"
#include <stdio.h>

char gps[100];

#define ACLK BIT2
#define MCLK BIT4
#define SMCLK BIT5

void gps_setup(void){
  gps[0] = 0x21; // "!" to indicate an unaltered register
  
//  // TODO don't output clocks in final design...
//  P1DIR |= ACLK + MCLK + SMCLK; // Pin 1.2 is output
//  P1SEL0 |= ACLK + MCLK + SMCLK; // Pin 1.2 output ACLK
//  P3SEL0 |= BIT0 + BIT1; // UCA0RXD + UCA0TXD

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

}


float gps_mintodeg(float tempmin)
{
  return tempmin / 60;
}


float gps_sectodeg(float tempsec)
{
  return tempsec / 3600;
}


void gps_RX(void)
{
  double timeh, timem, times, latd, latm, lond, lonm, lats, lons;
  int j;
  //sprintf(gps,"$GPGGA,225307.072,6451.3762,N,14749.1884,W,1,07,1.2,176.7,M,4.8,M,0.0,0000*57");
  for (j = 0; j < 100; j++) //change 42 to include alt
    {
      gps[j] = gps[j] - 0x30; // change all important ASCII chars to actual numbers...
    }
    // skip gps[0..5] = "$GPGGA" | skip gps[6] = ","
//    timeh = (gps[7] * 10) + gps[8];
//    timem = (gps[9] * 10) + gps[10];
//    times = (gps[11] * 10) + gps[12] + (gps[14] / 10) + (gps[15] / 100) + (gps[16] / 1000); // skip gps[13] = "."
    // skip gps[17] = ","
    latd = (gps[18] * 10) + gps[19];
    latm = (gps[20] * 10) + gps[21] + (gps[23] / 10) + (gps[24] / 100) + (gps[25] / 1000) + (gps[26] / 1000); // skip gps[22] = "."
    // skip gps[27] = ","
    lats = (gps[28] == ('N' - 0x30))? 1 : -1; // printf("%c\n\r", gps[28] + 0x30);
    apos[0] = lats * (latd + gps_mintodeg(latm)); // astronaut latitude
    // skip gps[29] = ","
    lond = (gps[30] * 100) + (gps[31] * 10) + gps[32];
    lonm = (gps[33] * 10) + gps[34] + (gps[36] / 10) + (gps[37] / 100) + (gps[38] / 1000) + (gps[39] / 10000); // skip gps[35] = "."
    // skip gps[40] = ","
    lons = (gps[41] == ('E' - 0x30))? 1 : -1; // printf("%c\n\r", gps[41] + 0x30);
    apos[1] = lons * (lond + gps_mintodeg(lonm)); // astronaut longitude
    // skip gps[42..51] = junk
    // alt = gps[52..?]
    apos[2] = 0; // astronaut altitude
}
