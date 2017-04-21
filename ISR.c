//ISR Routines go in here
//Created: 4/6/17
//Last update: 4/6/17

#include <msp430.h>
#include <msp430f6779a.h>
#include "IMU.h"
#include <stdio.h>
#include "pathfinding.h"
#include "gps.h"
#include <string.h>


//GPS ISR
void USCI_A0_ISR (void) __interrupt [USCI_A0_VECTOR]
{
  static int i = 0;

  switch(UCA0IV)
  {
    case USCI_UART_UCRXIFG:
      //Save current char into variable array
      gps[i] = UCA0RXBUF;
      //printf("%c\n\r", gps[i]);
      //Look for 'enter' at the end of the gps line
      if(gps[i] == '$')
      {
        //printf("%c\n\r", gps[i]);
        gps[0] = '$';
        i = 1;
        ////          |0    5|7       16|18     26| |30      39| | |  |   |52  56| |   | |   |       |
        //sprintf(gps,"$GPGGA,225307.072,6451.3762,N,14749.1884,W,1,07,1.2,176.7,M,4.8,M,0.0,0000*57\n\0"); //for testing
        //Check header (really all we care about is the GGA data, everything else can be dropped)
        if((gps[0] == '$') && (gps[1] == 'G') && (gps[2] == 'P') && (gps[3] == 'G') && (gps[4] == 'G') && (gps[5] == 'A')) // (!strcmp(gpsh,"$GPGGA"))
        {
          //printf("%s\n\r", gps);
          ctl_events_set_clear(&PF_events, GPS_EV, 0);
        }
      }
      else
      {
        i = (i + 1) % 100; // if not end of string, continue to count gps[] RX pointer
      }
    break;
    default: break;
  }
}


// IMU interrupt
void TimerA0_ISR (void) __interrupt [TIMER0_A1_VECTOR] {
  switch (TA0IV) {
    case TA0IV_TA0IFG:
      bno055_get_imu();
    break;
    default: break;
  }
}


