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
#include <SDlib.h>
#include <i2c.h>
#include <bno055.h> // for pin defines 
#include "IMU.h"
//*********************************************************** passing arguments over the terminal *********************************************
int example_command(char **argv,unsigned short argc){
  int i,j;
  //TODO replace printf with puts ? 
  printf("This is an example command that shows how arguments are passed to commands.\r\n""The values in the argv array are as follows : \r\n");
  for(i=0;i<=argc;i++){
    printf("argv[%i] = 0x%p\r\n\t""string = \"%s\"\r\n",i,argv[i],argv[i]);
    //print out the string as an array of hex values
    j=0;
    //test
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
    printf("Too many arguments.\r\n");
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
    printf("I2C error: NACK.\r\n");
    return resp;
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\r\n");
    return resp;
  }
  else {
    printf("I2C success.\r\n");
    return 0;
  }
}

int I2C_txrx(char **argv, unsigned short argc){
  unsigned char tx_buf[1],rx_buf[100],reg_addr;
  unsigned short addr, rx_len;
  short resp, i = 0;

  if (argc > 3){
    printf("Too many arguments.\n\r");
    printf("Usage: I2C_rx [addr] [reg addr] [# registers to read]");
    return -1;
  }

  addr=strtoul(argv[1], NULL, 0);        // grab i2c address 
  reg_addr=strtoul(argv[2], NULL, 0);    // register address and data to be written
  rx_len=strtoul(argv[3], NULL, 0);     // how much data to read
  
  resp = i2c_txrx(addr,&reg_addr, 1, rx_buf, rx_len);   // do i2c things

  if (resp == -1){
    printf("I2C error: NACK.\n\r");
    return resp;
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\n\r");
    return resp;
  }
  else if (resp>=0){
    while (rx_len > i){ // clock out rx_buf
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
/*
int pageID_cmd(char **argv, unsigned short argc){
  unsigned char rx_buf[100], tx_buf[1];
  unsigned short pageid, addr = BNO055_I2C_ADDR1;
  short resp;

 // resp = i2c_txrx(addr,&reg_addr, 1, rx_buf, rx_len);
  tx_buf[0] = BNO055_I2C_ADDR1;

  resp = i2c_txrx(addr, tx_buf, 2, rx_buf, 2);          //read existing page ID
  pageid = rx_buf[0];                                     //set page ID

  if (*argv[1] == 0x30|| *argv[1]== 0x31){             // check input args for a 1 or 0
    pageid= strtol(argv[1],NULL,0);                   // pars input
      tx_buf[0] = 0x01; // test

      resp = i2c_tx(addr, (unsigned char *) *argv[1], 1);   //  write new page ID  ... what will happen if i use an signed char insted of an unsigned char
   
    if(resp == 1){
      printf("PageID changed from 0x%x to 0x%x.\r\n",rx_buf[0],pageid);
    }
    else {
      printf("Erorr writing page ID.\r\nResponce = %i", resp);
    }
  }
  else{
    printf("PageID is set to 0x%x\r\n",pageid);
  }

  return 0;
}

int data_cmd(char **argv,unsigned short argc){

return 0;
}

*/

int pageID_cmd(char **argv, unsigned short argc){
  unsigned char rx_buff[2],tx_buff[1]={BNO055_PAGE_ID_ADDR};
  unsigned short pageid, addr;
  short resp;
 
  resp = i2c_txrx(addr, tx_buff, 1, rx_buff, 2);       //read existing page ID, read 2 bytes throw one away--> i2c errata

  if (*argv[1] == 0x30|| *argv[1] == 0x31){              // check input args for a 1 or 0
    pageid= strtoul(argv[1],NULL,0);                   // pars input is this bad ? 
    resp=bno055_pageid(addr, &pageid);                  //  write new page ID  
    
    if(resp == 1){
      resp = i2c_txrx(addr, tx_buff, 1, rx_buff, 2);       //read existing page ID, read 2 bytes throw one away--> i2c errata
      printf("PageID changed from 0x%x to 0x%x.\r\n",rx_buff[0],pageid);
    }
    else {
      printf("Erorr writing page ID.\r\nResponce = %i", resp);
    }
  }
  else{
    printf("PageID is set to 0x%x\r\n",rx_buff[0]);
  }

  return 0;
}

int data_cmd(char **argv,unsigned short argc){

return 0;
}

int setup_cmd(char **argv,unsigned short argc){ // note sensor settings can only be written when in configmode
  unsigned char tx_buf[2];
  unsigned short addr;
  short resp;

  addr=strtoul(argv[1], NULL, 0);          // save user input address
  tx_buf[0] = BNO055_OPR_MODE_ADDR;
  tx_buf[1] = BNO055_OPERATION_MODE_IMUPLUS;

  resp = i2c_tx(addr, tx_buf, 2);
  if (resp == -1){
    printf("I2C error: NACK.\r\n");
    return resp;
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\r\n");
    return resp;
  }
  else if (resp >= 0) {
    printf("I2C success.\r\n");
    return 0;
  }
  else {
    printf("Unknown I2C error: %i \r\n",resp);
    return resp;
  }
}

int reset_cmd(char **argv,unsigned short argc){
  unsigned char tx_buf[2];
  unsigned short addr;
  short resp;

  addr=strtoul(argv[1], NULL, 0);          // save user input address
  tx_buf[0] = BNO055_SYS_TRIGGER_ADDR;
  tx_buf[1] = BNO055_SYS_RST_MSK;

  resp = i2c_tx(addr, tx_buf, 2);
  if (resp == -1){
    printf("I2C error: NACK.\r\n");
    return resp;
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\r\n");
    return resp;
  }
  else if (resp >= 0) {
    printf("I2C success.\r\n");
    return 0;
  }
  else {
    printf("Unknown I2C error: %i \r\n",resp);
    return resp;
  }
}

int read_Quat(char **argv,unsigned short argc){
  unsigned char tx_buf[1],rx_buf[100],reg_addr;
  unsigned short addr, rx_len;
  short resp, i=0,j=0;

  if (argc > 3){
    printf("Too many arguments.\n\r");
    printf("Usage: I2C_rx [addr] [reg addr] [# registers to read]");
    return -1;
  }

  addr=strtoul(argv[1], NULL, 0);        // grab i2c address 
  reg_addr=BNO055_QUATERNION_DATA_W_LSB_ADDR;    // register address and data to be written
  rx_len=8;     // how much data to read
  
  /*for (i=0; i<10; i++) {
    resp = i2c_txrx(addr,&reg_addr, 1, rx_buf, rx_len);   // do i2c things
    if (resp < 1) break;
    else{
      while (rx_len > j){ // clock out rx_buf
        printf("Register address: 0x%X, Value: %X\n\r", reg_addr+j, rx_buf[j]);
        j++;
      }
    }
    i++;
    __delay_cycles(10000);
  }*/
  
  resp = i2c_txrx(addr,&reg_addr, 1, rx_buf, rx_len);   // do i2c things
  if (resp == -1){
    printf("I2C error: NACK.\n\r");
    return resp;
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\n\r");
    return resp;
  }
  else if (resp>=0){
    while (rx_len > j){ // clock out rx_buf
      printf("Register address: 0x%X, Value: %X\n\r", reg_addr+j, rx_buf[j]);
      j++;
    }
    printf("I2C success. returned %i\n\r",resp);
    return 0;
  }
  else{
    printf("Unknown Error, check wiki %i.\n\r",resp);
    return resp;
  }
}

// TODO pars more of the data ? 
int status_cmd(char **argv,unsigned short argc){
short resp;

resp = bno055_status();
if (resp == -1){
    printf("I2C error: NACK.\n\r");
    return resp;
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\n\r");
    return resp;
  }
  else if (resp>=0){
    printf("I2C success. returned %i args.\n\r",resp);      // check i2c args have been grabbed 
    printf("**************************************\r\n");
    printf("Calibration status is 0x%x.\n\r",glb_buff[0]);  //parsing calibration test
    if(glb_buff[0] == 0 ){
      printf("IMU is not calibrated.\r\n");}
    else if (glb_buff[0] > 0 ){
      printf("System calibration status is",glb_buff[0] &= BIT7+BIT6);
      printf("Gyro status is",glb_buff[0] &=  BIT5+BIT4);
      printf("ACC calibration status is",glb_buff[0] &= BIT3+BIT2);
      printf("MAG calibration status is",glb_buff[0] &= BIT1+BIT0);
    }
    else if(glb_buff[0] == 'f'){
      printf("IMU is calibrated.\r\n");}
    printf("**************************************\r\n");
    printf("ST_result is 0x%x.\n\r",glb_buff[1]); // Parsing Self test 
      if(glb_buff[1] == 0x0f){
        printf("All self tests passed.\r\n");}
      else{
        printf("Some self tests failed.\r\n");}
    printf("**************************************\r\n");
    printf("Int_sta is 0x%x.\n\r",glb_buff[2]); // check interrupt status of sensors
    printf("**************************************\r\n");
    if(glb_buff[2] != 0)
      printf("Some interrupt has been triggered.\r\n");
    else
    printf("No tinterrupts have been triggered.\r\n");
    printf("**************************************\r\n");
    printf("Sys_clk status is 0x%x.\n\r",glb_buff[3]);  // check system clock 
    printf("**************************************\r\n");
    printf("Sys_status is 0x%x.\n\r",glb_buff[4]);      // check over all system status 
    if(glb_buff[4] == 0)
      printf("IMU is idle.\r\n");
    else if(glb_buff[4] == 1)
      printf("IMU System Error.\r\n");  //TODO read 0x39 if this happens 
    else if(glb_buff[4] == 2) 
      printf("IMU is initializing paeripherals.\r\n");
    else if(glb_buff[4] == 3)
      printf("IMU is in System Initialization.\r\n");
    else if(glb_buff[4] == 4)
      printf("IMU is excuting selftest.\r\n");
    else if(glb_buff[4] == 5)
      printf("IMU is running sensor fusion algorithm.\r\n");
    else if(glb_buff[4] == 6)
      printf("IMU is running without fusion algorithm.\r\n");
    else
    return 0;
  }
  else{ // bad i2c_txrx 
    printf("Unknown Error, check wiki %i.\n\r",resp);
    return resp;
  }
return 0;
}

// reads and changes the op mode of the IMU 
int opperationmode_cmd(char **argv,unsigned short argc){


//short bno055_oprmode(mode);

 return 0;
}

//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd},
                   {"ex","[arg1] [arg2] ...\r\n\t""Example command to show how arguments are passed",example_command},
                   {"i2c_tx","Usage: I2C_tx [addr (0x28)] [reg addr] [data]\n\r",I2C_tx},
                   {"i2c_txrx"," Usage: I2C_rx [addr (0x28)] [reg addr] [# registers to read]\n\rDefault IMU adress is 0x28.\n\r",I2C_txrx},
                   {"data","get sum data?",data_cmd},
                   {"setup","get sum data?",setup_cmd},
                   {"reset","Reset IMU", reset_cmd},
                   {"pageid","checks page ID and changes page ID if passed an arg.\n\r",pageID_cmd},
                   {"quat","Reads all quaternion data registers", read_Quat},
                   {"status","Reads all relavtent IMU status registers", status_cmd},
                   {"opp","Reads all relavtent IMU status registers", opperationmode_cmd},


                   //ARC_COMMANDS,CTL_COMMANDS,ERROR_COMMANDS, // add lib functions to the help list 
                   //end of list
                   {NULL,NULL,NULL}};
