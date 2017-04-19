//ISR Routines go in here
//Created: 4/6/17
//Last update: 4/6/17

#include <msp430.h>
#include <msp430f6779a.h>
#include <IMU.h>
#include <stdio.h>
#include <pathfinding.h>
#include <gps.h>
#include <string.h>


//GPS ISR
/*#pragma vector = USCI_A0_VECTOR
__interrupt*/
void USCI_A0_ISR (void) __interrupt [USCI_A0_VECTOR]
{
  static int i = 0;
  //GPS data & gps header
  //char gps[500];
  //char gpsh[6];
  //int comma = 0;
  //int cycle, deg, min, sec;
  //double GpsLat, GpsLong, GpsElv;

  float timeh, timem, times, latd, latm, lats, lond, lonm, lons;
  int j;
  j = 0; // mostly for break point.

  switch(UCA0IV)
  {
    case USCI_UART_UCRXIFG:
      //Save current char into variable array
      gps[i] = UCA0RXBUF;
      //printf("%c", gps[i]);
      //Look for 'enter' at the end of the gps line
      if(gps[i] == 0x0D)
      {
        i = 0;
        ////          |0    5|7       16|18     26| |30      39| | |  |   |52  56| |   | |   |       |
        //sprintf(gps,"$GPGGA,225307.072,6451.3762,N,14749.1884,W,1,07,1.2,176.7,M,4.8,M,0.0,0000*57\n\0"); //for testing
        //Check header (really all we care about is the GGA data, everything else can be dropped)
        if((gps[0] == '$') && (gps[1] == 'G') && (gps[2] == 'P') && (gps[3] == 'G') && (gps[4] == 'G') && (gps[5] == 'A')) // (!strcmp(gpsh,"$GPGGA"))
        {
          for (j = 0; j < 42; j++) //change 42 to include alt
          {
            gps[j] = gps[j] - 0x30; // change all important ASCII chars to actual numbers...
          }
          // skip gps[0..5] = "$GPGGA" | skip gps[6] = ","
//          timeh = (gps[7] * 10) + gps[8];
//          timem = (gps[9] * 10) + gps[10];
//          times = (gps[11] * 10) + gps[12] + (gps[14] / 10) + (gps[15] / 100) + (gps[16] / 1000); // skip gps[13] = "."
          // skip gps[17] = ","
          latd = (gps[18] * 10) + gps[19];
          latm = (gps[20] * 10) + gps[21]; // + (gps[23] / 10) + (gps[24] / 100) + (gps[25] / 1000) + (gps[26] / 1000); // skip gps[22] = "."
          lats = (gps[23] * 1000) + (gps[24] * 100) + (gps[25] * 10) + gps[26];
          apos[0] = latd + gps_mintodeg(latm) + gps_sectodeg(lats); // astronaut latitude
          // skip gps[27] = ","
//          lats = (gps[28] == "N")? 1 : -1;
          // skip gps[29] = ","
          lond = (gps[30] * 10) + gps[31];
          lonm = (gps[32] * 100) + (gps[33] * 10) + gps[34] + (gps[36] / 10) + (gps[37] / 100) + (gps[38] / 1000) + (gps[39] / 10000); // skip gps[35] = "."
          apos[1] = lond + gps_mintodeg(lonm); // astronaut longitude
          // skip gps[40] = ","
//          lons = (gps[41] == "N")? 1 : -1;
          // skip gps[42..51] = junk
          // alt = gps[52..?]
          apos[2] = 0; // astronaut altitude
          ctl_events_set_clear(&PF_events, GPS_EV, 0);
        }
      }
      else
      {
        i = (i + 1) % 42; // if not end of string, continue to count gps[] RX pointer
      }
    break;
    default: break;
  }
}



//void TimerA0_ISR (void) __interrupt [TIMER0_A0_VECTOR] {
//  switch (TA0IV) {
//    case TA0IV_TA0IFG:
//      bno055_get_euler();
//      break;
//
//    default:
//      break;
//  }
//}


