//ISR Routines go in here
//Created: 4/6/17
//Last update: 4/6/17

#include <msp430.h>
#include <msp430f6779a.h>

//GPS ISR
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR (void)
  {
     static int i=0;
     //GPS data
     char gps[100], txgps;

    switch(UCA0IV)
      {
      case 0x00: break;

      case 0x02:
       //Save current char into variable array
       gps[i] = UCA0RXBUF;
       gps[i+1] = '\0';
             
       //Look for 'enter' at the end of the gps line
       if(gps[i] == '\n')
        {
          UCA2TXBUF = '\n';             // start a new line
            while(!(UCA2IFG & UCTXIFG));  // delay for UART TX
            
            i = 0;        // reset global counter       
            while(gps[i] != '\0')
            {
              //Print GPS back to the terminal,
              UCA2TXBUF = gps[i];
              while(!(UCA2IFG & UCTXIFG));  // delay for UART TX
              i++;  // increment counter
            }
            i = 0;
        }
       else
        {
          i++;
        }
        break;

      case 0x04: break;
      default: break;
      }
  }
