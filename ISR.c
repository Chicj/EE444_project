//ISR Routines go in here
//Created 4/6/17
//Last update: 4/6/17

#include <msp430.h>
#include <msp430f6779a.h>

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR (void)
//void USCI_A0_isr(void) __interrupt[USCI_A0_VECTOR]
  {
    static int i=0;
           int a,b;
    switch(UCA0IV)
      {
      case 0x00: break;
      case 0x02:
       //UCA0TXBUF = UCA0RXBUF;
        UCA0IFG &= ~UCTXIFG;
        i = i+1;
        
        switch(UCA0RXBUF)
          {
            case 13:
              i=i-1;
              if(i<9)
                {
                UCA1TXBUF = i+'0';
                i=0;
                break;
                }
              else if(i>9)
                {
                a = i/10;
                UCA0TXBUF = a+'0';
                UCA0TXBUF = i+'0';
                }
          }
        break;
      case 0x04: break;
      default: break;
      }
  }
