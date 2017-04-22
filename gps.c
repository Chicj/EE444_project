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
  
  // TODO don't output clocks in final design...
  P1DIR |= ACLK + MCLK + SMCLK; // Pin 1.2 is output
  P1SEL0 |= ACLK + MCLK + SMCLK; // Pin 1.2 output ACLK
  P3SEL0 |= BIT0 + BIT1; // UCA0RXD + UCA0TXD

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


double gps_mintodeg(double tempmin)
{
  return tempmin / 60.0;
}


void gps_RX(void)
{
  int j;
  //sprintf(gps,"$GPGGA,225307.072,6451.3762,N,14749.1884,W,1,07,1.2,176.7,M,4.8,M,0.0,0000*57");
  for (j = 0; j < 100; j++) //change 42 to include alt
    {
      gps[j] = gps[j] - 0x30; // change all important ASCII chars to actual numbers...
    }
    // time = gps_time();
    apos[0] = gps_lat();
    apos[1] = gps_lon();
    apos[2] = gps_alt();
}


//float gps_time(void)
//{
//  // skip gps[0..5] = "$GPGGA" | skip gps[6] = ","
//  timeh = (gps[7] * 10) + gps[8];
//  timem = (gps[9] * 10) + gps[10];
//  times = (gps[11] * 10) + gps[12] + (gps[14] / 10) + (gps[15] / 100) + (gps[16] / 1000); // skip gps[13] = "."
//  // skip gps[17] = ","
//}


double gps_lat(void)
{
  //float latd, latm, latsin;
  int latd;
  float latm;
  char latsin;

  // skip gps[17] = ","
  latd = gps_latd(); // (gps[18] * 10.0) + gps[19];
  latm = gps_latm(); // (gps[20] * 10.0) + gps[21] + (gps[23] / 10.0) + (gps[24] / 100.0) + (gps[25] / 1000.0) + (gps[26] / 10000.0); // skip gps[22] = "."
  // skip gps[27] = ","
  latsin = (gps[28] == ('N' - 0x30))? 1 : -1; // printf("%c\n\r", gps[28] + 0x30);
  return latsin * (latd + gps_mintodeg(latm)); // astronaut latitude
}

double gps_lon(void)
{
  double lond, lonm, lonsin;
  // skip gps[29] = ","
  lond =  gps_lond(); // (gps[30] * 100.0) + (gps[31] * 10.0) + gps[32];
  lonm =  gps_lonm(); // (gps[33] * 10.0) + gps[34] + (gps[36] / 10.0) + (gps[37] / 100.0) + (gps[38] / 1000.0) + (gps[39] / 10000.0); // skip gps[35] = "."
  // skip gps[40] = ","
  lonsin = (gps[41] == ('E' - 0x30))? 1 : -1; // printf("%c\n\r", gps[41] + 0x30);
  return lonsin * (lond + gps_mintodeg(lonm)); // astronaut longitude
}

double gps_alt(void)
{
  // skip gps[42..51] = junk
  // alt = gps[52..?]
  return 0; // astronaut altitude
}

double gps_latd(void)
{
  return (gps[18] * 10.0) + gps[19];
}

double gps_latm(void)
{
  double var0 = (gps[20] * 10.0);
  double var1 = gps[21];
  double var2 = (gps[23] / 10.0);
  double var3 = (gps[24] / 100.0);
  double var4 = (gps[25] / 1000.0);
  double var5 = (gps[26] / 10000.0); // skip gps[22] = "."
  return var0 + var1 + var2 + var3 + var4 + var5;
}

double gps_lond(void)
{
  return (gps[30] * 100.0) + (gps[31] * 10.0) + gps[32];
}

double gps_lonm(void)
{
  double var0 = (gps[33] * 10.0);
  double var1 = gps[34];
  double var2 = (gps[36] / 10.0);
  double var3 = (gps[37] / 100.0);
  double var4 = (gps[38] / 1000.0);
  double var5 = (gps[39] / 10000.0); // skip gps[35] = "."
  return var0 + var1 + var2 + var3 + var4 + var5;
}
