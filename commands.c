/**********************************************************************************************************************************************
The commands.c file is for commands that will be displayed through the serial terminal. 
In order to add a command you must create a function as seen below.
Then function must be added to the "const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd}" table at the end of the file.
**********************************************************************************************************************************************/
#include <msp430.h>   // prune these 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <terminal.h>
#include <commandLib.h>
#include <stdlib.h>
#include <ARCbus.h>
#include <SDlib.h>
#include <i2c.h>

//*********************************************************** passing arguments over the terminal *********************************************
int example_command(char **argv,unsigned short argc){
  int i,j;
  //TODO replace printf with puts ? 
  printf("This is an example command that shows how arguments are passed to commands.\r\n""The values in the argv array are as follows : \r\n");
  for(i=0;i<=argc;i++){
    printf("argv[%i] = 0x%p\r\n\t""string = \"%s\"\r\n",i,argv[i],argv[i]);
    //print out the string as an array of hex values
    j=0;
    printf("\t""hex = {");
    do{
      //check if this is the first element
      if(j!='\0'){
        //print a space and a comma
        printf(", ");
      }
      //print out value
      printf("0x%02hhX",argv[i][j]);
    }while(argv[i][j++]!='\0');
    //print a closing bracket and couple of newlines
    printf("}\r\n\r\n");
  }
  return 0;
}

int I2C_tx(char **argv, unsigned short argc){
  unsigned char tx_buf[2];
  unsigned short addr;
  short resp;

  if (argc > 4) {
    printf("Too many arguments.\n");
    printf("Usage: I2C_tx [addr] [reg addr] [data]");
    return -1;
  }

  // I2C address
  addr=strtoul(argv[1], NULL, 0);
  // register address and data to be written
  tx_buf[0]=strtoul(argv[2], NULL, 0);
  tx_buf[1]=strtoul(argv[3], NULL, 0);

  resp = i2c_tx(addr, tx_buf, 2);
  if (resp == -1){
    printf("I2C error: NACK.\n");
    return resp;
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\n");
    return resp;
  }
  else {
    printf("I2C success.\n");
    return 0;
  }
}

int I2C_txrx(char **argv, unsigned short argc){
  unsigned char tx_buf[1];
  unsigned char rx_buf[100];
  unsigned short addr, reg_addr, rx_len;
  short resp, i = 0;

  if (argc > 3){
    printf("Too many arguments.\n\r");
    printf("Usage: I2C_rx [addr] [reg addr] [# registers to read]");
    return -1;
  }

  // I2C address
  addr=strtoul(argv[1], NULL, 0);
  // register address and data to be written
  reg_addr=strtoul(argv[2], NULL, 0);
  rx_len=strtoul(argv[3], NULL, 0);

  resp = i2c_txrx(addr, tx_buf, 1, rx_buf, rx_len);
  if (resp == -1){
    printf("I2C error: NACK.\n\r");
    return resp;
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\n\r");
    return resp;
  }
  else if (resp>=0){
    rx_buf[rx_len] = '\0';
    while (rx_buf[i] != '\0'){
      printf("Register address: 0x%X, Value: %X\n\r", reg_addr+i, rx_buf[i]);
      i++;
    }
    printf("I2C success. returned %i\n\r",resp);
    return 0;
    
  }
  else{
    printf("Unknown Error, check wiki %i.\n\r",resp);
    return resp;
  }
}



//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd},
                   {"ex","[arg1] [arg2] ...\r\n\t""Example command to show how arguments are passed",example_command},
                   {"i2c_tx","stuff_tx.\n\rDefault IMU adress is 0x28.\n\r",I2C_tx},
                   {"i2c_txrx","stuff_txrx.\n\rDefault IMU adress is 0x28.\n\r",I2C_txrx},
                   ARC_COMMANDS,CTL_COMMANDS,ERROR_COMMANDS, // add lib functions to the help list 
                   //end of list
                   {NULL,NULL,NULL}};

