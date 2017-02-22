#include <msp430.h>
#include "bno055.h"

// function prototypes 
extern int IncrementVcore(void);
extern int DecrementVcore (void);


void main(void)
{
//***************************************************************Clock set up*****************************
// set PM core to acceptable value for 16 MHz pg16 data sheet
  DecrementVcore();
  DecrementVcore();
  DecrementVcore();

  IncrementVcore();
  IncrementVcore();  // ensure operation in PMM lvl 2 in in order to run 16 MHz clk

// set SMCLK to 16 MHz (default SELREF for FLLREFCLK is XT1CLK = 32*1024 = 32768 Hz = 32.768 KHz)
  UCSCTL2 = 488;                  // Setting the freq multiplication factor * 1024 for final clk freq @ 16 MHz
  UCSCTL1 |= DCORSEL_4;         // This sets the DCO frequency pg26 data sheet 12.3 -- 28.3 MHz
  UCSCTL4 |= SELS__DCOCLKDIV ;  // set output of FLLREFCLK --> DCOCLKDIV to input of SMCLK  <-- This is default 
//*************************************************************** pin set up ***************************
  // setup breakout board MCLK and SMCLK test pins for scope testing
  P11DIR  |= BIT2;  // Set Port 11 pin 2 direction as an output
  P11SEL |= BIT2;   // Set port 11 pin 2 as SMCLK signal

  //setting up pins for SW1 --> P2.6
  P2REN |= BIT6 + BIT7;  // turn on internal resistor to combat bounce (this pulls down)
  P2OUT |= BIT6 + BIT7;  // Pull up when P2REN is high for both switches ( this pulls up), pg 410 fam
  P2IE |= BIT6 + BIT7;   // Enable IR on P2.6 and P2.7

  P1OUT |= BIT0 + BIT1;  // setup test LED's
  P1DIR |= BIT0 + BIT1;  // Set LED High on start up   

//************************************************************* I2C set up [UCB1] ****************************

  _EINT();  // set global IR enable 
  LPM0;
}

 //button IR code
void Button_IR(void) __interrupt[PORT2_VECTOR]{
int i=0 ,k=0;
        switch(P2IV)
        {
          case P2IV_NONE: 
          break; 
          case P2IV_P2IFG0:
          break; 
          case P2IV_P2IFG1: 
          break; 
          case P2IV_P2IFG2: 
          break; 
          case P2IV_P2IFG3: 
          break; 
          case P2IV_P2IFG4:
          break; 
          case P2IV_P2IFG5: 
          break; 
          case P2IV_P2IFG6: // check button flag, SW1 on breakout board
             P1DIR ^= BIT0;  // toggle LED when button push
             
          break; 
          case P2IV_P2IFG7:// check button , SW2 on breakout board 
          break; 
        }
}


//I2C data ISR, called to transmit or receive I2C data
void I2C_dat(void) __interrupt[USCI_B1_VECTOR]{
  switch(UCB1IV){
    case USCI_I2C_UCNACKIFG:
      //Not-Acknowledge received
      UCB1STAT&=~UCNACKIFG;     //clear interrupt flag
      UCB1CTLW0|=UCTXSTP;       //generate stop condition
    case USCI_I2C_UCRXIFG:
      //receive data
    break;
    case USCI_I2C_UCTXIFG:
      //check if there are more bytes
    break;
  }  
} 

//**************** TODO ******************************
// start a lib for this project to allow more concise code 
// - timer .h and .c to set SMCLK to 16 MHz as a main start up function.
//  - make an I2C lib ? 
//
//*****************************************************
