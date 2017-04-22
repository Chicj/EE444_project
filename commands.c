/**********************************************************************************************************************************************
The commands.c file is for commands that will be displayed through the serial terminal. 
In order to add a command you must create a function as seen below.
Then function must be added to the "const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd}" table at the end of the file.
**********************************************************************************************************************************************/
#include <msp430.h>     // prune these 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <terminal.h>
#include <commandLib.h>
#include <stdlib.h>
#include <SDlib.h>
#include <i2c.h>
#include <bno055.h>     // for pin defines 
#include <UCA2_uart.h>  // for check keys
#include <ctl.h>        // for delay
#include "IMU.h"
#include "pathfinding.h"
#include <math.h>
#include "gps.h"
#include "LED.h"

#ifndef  M_PI 
#define  M_PI 3.1415926535897932384/* pi  for problems with Crossworks*/
#endif
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


int spoofGPS(char **argv, unsigned short argc)
{
  // argv[0] = "sgps";
  float templat = strtof(argv[1],NULL);
  float templon = strtof(argv[2],NULL);
  float tempalt = strtof(argv[3],NULL);

  float trgt = tWP;

  if (argc > 3) {
    printf("Too many arguments.\r\n");
    printf("Usage: sgps [lat] [lon] [alt]");
    return -1;
  }
  else
  {
    printf("\n\r");

    pathfindGPS (templat, templon, tempalt);
  
    printf("Distance: %f m\n\r", pathfindDistance());
  
    pathfindTarget();
  
    if (trgt != tWP)
    {
      printf("WAYPOINT REACHED!\n\r");
      printf("New Distance: %f m\n\r", pathfindDistance());
      printf("New Target: %i %f %f %f\n\r", tWP, tpos[0], tpos[1], tpos[2]); // print next target waypoint location
    } 
    else
    {
      printf("Target: %i %f %f %f\n\r", tWP, tpos[0], tpos[1], tpos[2]); // print next target waypoint location
    }
  
    pathfindHeading();
    printf("Rotate: %3.1f deg\n\r", pathfindPoint());

    printf("\n\r");

    return 0;
  }
}


int spoofIMU(char **argv, unsigned short argc)
{
  // argv[0] = "simu";
  float temphed = strtof(argv[1],NULL);
  
  if (argc > 1) {
    printf("Too many arguments.\r\n");
    printf("Usage: simu [hed]");
    return -1;
  }
  else
  {
    printf("\n\r");

    pathfindIMU(temphed);
    printf("Rotate: %3.1f deg\n\r", pathfindPoint());

    printf("\n\r");

    return 0;
  }
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


int pageID_cmd(char **argv, unsigned short argc){
  unsigned char rx_buff[2],tx_buff[1]={BNO055_PAGE_ID_ADDR};
  unsigned short pageid, addr;
  short resp;
 
  resp = i2c_txrx(addr, tx_buff, 1, rx_buff, 2);       //read existing page ID, read 2 bytes throw one away--> i2c errata

  if (*argv[1] == 0x30|| *argv[1] == 0x31){              // check input args for a 1 or 0
    pageid= strtoul(argv[1],NULL,0);                   // pars input is this bad ? 
    resp=bno055_pageid(pageid);                  //  write new page ID  NOPPEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE!!!!!!!!!!!!!!
    
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
  short resp;
  resp = bno055_reset();
  if (resp >= 0) {
    printf("I2C success.\r\n");
    return 0;
  }
  else if (resp == -1){
    printf("I2C error: NACK.\r\n");
    return resp;
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\r\n");
    return resp;
  }
  else {
    printf("Unknown I2C error: %i \r\n",resp);
    return resp;
  }
}


int read_Quat(char **argv,unsigned short argc){
  unsigned char reg_addr = 0x20;
  short resp, j=0;
  unsigned int count;

  if (argc != 0){
    printf("Usage: quat\n\r");
    return -1;
  }
  while(UCA2_CheckKey()==EOF){
    resp = bno055_get_quat();
    if (resp >= 0){
      while (8 > j){ // clock out rx_buf
        printf("Register address: 0x%X, Value: %X\n\r", reg_addr+j, glb_buff[j]);
        j++;
      }
      j = 0;
      printf("I2C success. returned %i\n\r",resp);
    }
    else if (resp == -1){
      printf("I2C error: NACK.\n\r");
    }
    else if (resp == -2){
      printf("I2C error: Timeout.\n\r");
    }

    else{
      printf("Unknown Error, check wiki %i.\n\r",resp);
    }
    count = 0;
    while (count < 65535){count++;}
  }
  return resp;
}


int read_Euler(char **argv, unsigned short argc){ // NOT WORKING YET!
  unsigned char reg_addr = 0x20;
  short resp, j=0;
  unsigned int count = 0;
  float euler[3];
  int reg[3];


  if (argc != 0){
    printf("Usage: euler\n\r");
    return -1;
  }

  while(UCA2_CheckKey()==EOF){

    resp = bno055_get_euler();
    if (resp >= 0){      
      // 1 degree = 16 LSB, 1 radian = 900 LSB
      euler[0] = ((eul_buff[1] << 8) + eul_buff[0]) / 16.0; // e[x] 0 to 360
      euler[1] = ((eul_buff[3] << 8) + eul_buff[2]) / 16.0; // e[y] 0 to 360
      euler[2] = ((eul_buff[5] << 8) + eul_buff[4]) / 16.0; // e[z] 0 to 360
      
      euler[0] = (euler[0] > 180)? euler[0] - 360 : euler[0]; // e[x] -180 to 180 (angle > 180)? angle - 360 : angle
      euler[1] = (euler[1] > 180)? euler[1] - 360 : euler[1]; // e[y] -180 to 180
      euler[2] = (euler[2] > 180)? euler[2] - 360 : euler[2]; // e[z] -180 to 180
      
      printf("\n\r");
      printf("hd: % 5.1f | at: % 5.1f | bk: % 5.1f\n\r", euler[0], euler[1], euler[2]);
    }
    else if (resp == -1){
      printf("I2C error: NACK.\n\r");
    }
    else if (resp == -2){
      printf("I2C error: Timeout.\n\r");
    }
    else{
      printf("Unknown Error, check wiki %i.\n\r",resp);
    }

    count = 0;
    while (count < 65535){count++;} // pause for a little bit... 65535 max for unsigned int
  }
  return resp;
}


// TODO pars more of the data ? 
int status_cmd(char **argv,unsigned short argc){
  short resp;
  resp = bno055_status();

  if (resp >= 0){
    printf("I2C success. returned %i args.\n\r",resp);      // check i2c args have been grabbed 

    printf("**************************************\r\n");
    printf("CALIB_STAT is 0x%x.\n\r",glb_buff[0]);  //parsing calibration test
    if(glb_buff[0] == 0 ){
      printf("IMU is not calibrated.\r\n");
    }
    else if(glb_buff[0] == 0xf){
      printf("IMU is fully calibrated.\r\n");
    }
    else{
      printf("System calibration status is: %d\n\r",(glb_buff[0] & BIT7+BIT6)>>6);
      printf("Gyro status is: %d\n\r",(glb_buff[0] &  BIT5+BIT4)>>4);
      printf("ACC calibration status is: %d\n\r",(glb_buff[0] & BIT3+BIT2)>>2);
      printf("MAG calibration status is: %d\n\r",glb_buff[0] & BIT1+BIT0);
    }

    printf("**************************************\r\n");
    printf("ST_RESULT is 0x%x.\n\r",glb_buff[1]); // Parsing Self test 
    if(glb_buff[1] == 0x0f){
      printf("All self tests passed.\r\n");
    }
    else if (glb_buff[1] == 0x00){
      printf("All self tests failed.\r\n");
    }
    else {
      printf("Microcontroller self test result: %s.\n\r", (glb_buff[1] & BIT3) ? "Passed" : "Failed");
      printf("Gyroscope self test result: %s.\n\r", (glb_buff[1] & BIT2) ? "Passed" : "Failed");
      printf("Magnetometer self test result: %s.\n\r", (glb_buff[1] & BIT1) ? "Passed" : "Failed");
      printf("Accelerometer self test result: %s.\n\r", (glb_buff[1] & BIT0) ? "Passed" : "Failed");
    }

    printf("**************************************\r\n");
    printf("INT_STA is 0x%x.\n\r",glb_buff[2]); // check interrupt status of sensors
    if(glb_buff[2] != 0){
      printf("Some interrupt has been triggered.\r\n");
    }
    else{
      printf("No interrupts have been triggered.\r\n");
    }

    printf("**************************************\r\n");
    printf("SYS_CLK_STATUS is 0x%x.\n\r",glb_buff[3]);  // check system clock

    printf("**************************************\r\n");
    printf("SYS_STATUS is 0x%x.\n\r",glb_buff[4]);      // check over all system status
    printf("System status Message: %s.\n\r",sys_status_strings[glb_buff[4]]);

    printf("**************************************\r\n");
    printf("SYS_ERR is 0x%x.\n\r",glb_buff[5]);      // check over all system status
    printf("System error message: %s.\n\r",sys_err_strings[glb_buff[5]]);
  }
  else if (resp == -1){
    printf("I2C error: NACK.\n\r");
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\n\r");
  }
  else{ // bad i2c_txrx 
    printf("Unknown Error, check wiki %i.\n\r",resp);
  }
  return resp;
}


// reads and changes the op mode of the IMU 
int get_oprcmd(char **argv,unsigned short argc){
  short resp;
  resp = bno055_get_oprmode();

  if (resp > 0){
    printf("OPR_MODE is: %s.\n\r", opr_mode_strings[glb_buff[0]]);
    printf("PWR_MODE is: %s.\n\r", pwr_mode_strings[glb_buff[1]]);
  }
  else if (resp == -1){
    printf("I2C error: NACK.\n\r");
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\n\r");
  }
  else{ // bad i2c_txrx 
    printf("Unknown Error, check wiki %i.\n\r",resp);
  }
  return resp;
}


int set_oprmode_default(char **argv, unsigned short argc){
  short resp;
  resp = bno055_set_oprmode_default();
  if (resp > 0){
    printf("Opr mode set to IMU.\n\r"); 

  }
  else if (resp == -1){
    printf("I2C error: NACK.\n\r");
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\n\r");
  }
  else{ // bad i2c_txrx 
    printf("Unknown Error, check wiki %i.\n\r",resp);
  }
  return resp;
}


int set_oprmode(char **argv, unsigned short argc){
  short resp, opr_mode, i;
  if (argc != 2) {
    printf("Usage: set_opr_mode [opr_mode#].\n");
    for (i = 0; i < 13; i++){
      printf("Operating mode #%d: %s\n\r",i,opr_mode_strings[i]);
    }
  }
  opr_mode = strtoul(argv[1], NULL, 0);
  resp = bno055_set_oprmode(opr_mode);

  if (resp > 0){
    printf("Opr mode set to %s.\n\r");
  }
  else if (resp == -1){
    printf("I2C error: NACK.\n\r");
  }
  else if (resp == -2){
    printf("I2C error: Timeout.\n\r");
  }
  else{ // bad i2c_txrx 
    printf("Unknown Error, check wiki %i.\n\r",resp);
  }
  return resp;
}


// spoof gps, then use actual IMU data.
int testIMU (char **argv, unsigned short argc)
{
  float templat;
  float templon;
  float tempalt;
  float temphed;
  short resp;
  unsigned int count;

  if (argc > 3) {
    printf("Too many arguments.\r\n");
    printf("Usage: timu [lat] [lon] [alt]");
    return -1;
  }
  else
  {
    // argv[0] = "sgps";
    templat = strtof(argv[1],NULL);
    templon = strtof(argv[2],NULL);
    tempalt = strtof(argv[3],NULL);

    while(UCA2_CheckKey()==EOF){
      resp = bno055_get_euler();
      if (resp >= 0){      
        // 1 degree = 16 LSB, 1 radian = 900 LSB
        temphed = ((eul_buff[1] << 8) + eul_buff[0]) / 900.0; // euler[heading] from 0 to 2pi
        temphed = (temphed > M_PI)? temphed - (2 * M_PI) : temphed; // euler[heading] from -pi to pi

        //printf("euler dir:  hd: % 6.1f | at: % 6.1f | bk: % 6.1f\n\r", euler[0] - 180, euler[1] - 180, euler[2] - 180);
      
        pathfindGPS (templat, templon, tempalt);
        pathfindTarget();
        pathfindIMU(temphed);
        pathfindHeading();

        printf("\n\r");
        //printf("%c[2J", 27); // clear terminal
        //printf("thed: % 5.1f degrees\n\r", thed * 180 / M_PI);
        //printf("ahed: % 5.1f degrees\n\r", ahed * 180 / M_PI);
        printf("Rotate: % 5.1f degrees\n\r", pathfindPoint());
      }
      else if (resp == -1){
        printf("I2C error: NACK.\n\r");
      }
      else if (resp == -2){
        printf("I2C error: Timeout.\n\r");
      }
      else{
        printf("Unknown Error, check wiki %i.\n\r",resp);
      }

      count = 0;
      while (count < 65535){count++;} // pause for a little bit, 65535 max for unsigned int

    }
    return resp;
  }
}


// test i2c, make sure it is printing properly, it seems fine
int testI2C (char **argv, unsigned short argc)
{
  unsigned int count;
  if (argc > 2) {
    printf("Usage: ti2c [str (no spaces)]\n");
  }
  else{
    while(UCA2_CheckKey()==EOF){
      printf("%s\n\r", argv[1]);
      count = 0;
      while (count < 65535){count++;}
    }
  }
  return 0;
}


//// test IMU tasking
//int testTaskingGPS (char **argv, unsigned short argc)
//{
//  unsigned int count;
//  if (argc > 4) {
//    printf("Usage: ttimu\n");
//  }
//  else{
//    while(UCA2_CheckKey()==EOF){
//      bno055_get_imu();
//      count = 0;
//      while (count < 65535){count++;}
//    }
//  }
//  return 0;
//
//  float templat;
//  float templon;
//  float tempalt;
//  float temphed;
//  short resp;
//  unsigned int count;
//
//  if (argc > 3) {
//    printf("Too many arguments.\r\n");
//    printf("Usage: timu [lat] [lon] [alt]");
//    return -1;
//  }
//  else
//  {
//    // argv[0] = "sgps";
//    templat = strtof(argv[1],NULL);
//    templon = strtof(argv[2],NULL);
//    tempalt = strtof(argv[3],NULL);
//}


// test IMU tasking
int testTaskingIMU (char **argv, unsigned short argc)
{
  unsigned int count;
  if (argc > 1) {
    printf("Usage: ttimu\n");
  }
  else{
    while(UCA2_CheckKey()==EOF){
      bno055_get_imu();
      count = 0;
      while (count < 65535){count++;}
    }
  }
  return 0;
}


// read current gps[] string
int readGPS (char **argv, unsigned short argc)
{
  if (argc > 1) {
    printf("Usage: ttimu\n");
  }
  else{
    printf("%s\n\r", gps);
  }
  return 0;
}


// calls TX interupt to transmit "U" ASCII symbol (binary 01010101)
int testTX (char **argv, unsigned short argc)
{
  unsigned int count;
  if (argc > 1) {
    printf("Usage: ttimu\n");
  }
  else{
    while(UCA2_CheckKey()==EOF){
      UCA0TXBUF = 0x55;
      //count = 0;
      //while (count < 65535){count++;}
    }
  }
  return 0;
}


// TODO for some reason commands.c is missing prototype for write_LED()!
// calls TX interupt to transmit "U" ASCII symbol (binary 01010101)
int testLED_cmd (char **argv, unsigned short argc)
{
  char i = 0;
  int input = 1 , k;
  //input=strtol(argv[1],NULL,0);// populate LED input
  // write_LED(input);  // will command LEDs
  if(argc == 1)
  {
   input=strtol(argv[1],NULL,0);// populate LED input//
   write_LED(input);  // will command LEDs
  }
  else
  {
    while(UCA2_CheckKey()==EOF)
    {
      for(k=0;k<10000;k++){};
      input = input<<1; //pulse all LEDS
      write_LED(input);  // will command LEDs
      i++;
      if(i>15)
      {
        input = 1;
        i=0;
      }
    }
  }
  return 0;
}


int start_timer (char **argv, unsigned short argc)
{
  TA0CTL = TASSEL__ACLK + MC__UP + TAIE;
}


//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd},
                   {"ex","[arg1] [arg2] ...\r\n\t""Example command to show how arguments are passed",example_command},
                   {"i2c_tx","Usage: I2C_tx [addr (0x28)] [reg addr] [data]\n\r",I2C_tx},
                   {"i2c_txrx"," Usage: I2C_rx [addr (0x28)] [reg addr] [# registers to read]\n\rDefault IMU adress is 0x28.\n\r",I2C_txrx},
                   {"setup","get sum data?",setup_cmd},
                   {"reset","Reset IMU", reset_cmd},
                   {"pageid","checks page ID and changes page ID if passed an arg.\n\r",pageID_cmd},
                   {"quat","Reads all quaternion data registers\n\r", read_Quat},
                   {"euler","Reads all quaternion data registers and converts them to euler angles\n\r", read_Euler},
                   {"stat","Reads all relavtent IMU status registers\n\r", status_cmd},
                   {"readop","Reads IMU operation mode and power mode\n\r", get_oprcmd},
                   {"setopdef","Set IMU operation mode to default (IMU mode)\n\r", set_oprmode_default},
                   {"setOp","Set IMU operation mode to passed num\n\r", set_oprmode},
                   {"sgps","sgps [lat] [lon] [alt]""Spoofs astronaut's GPS coordinates.\n\r", spoofGPS},
                   {"simu","simu [hed]""Spoofs astronaut's IMU heading.\n\r", spoofIMU},
                   {"timu","timu [lat] [lon] [alt]""Spoofs astronaut's GPS coordinates, but uses actual IMU data.\n\r", testIMU},
                   {"ti2c","ti2c [str (no spaces)]""returns input str (no spaces) forever, unless key is pressed.\n\r", testI2C},
//                   {"ttgps","ttgps [lat] [lon] [alt]""activates GPS_EV once, uses lat, lon, and alt arguments", testTaskingGPS},
                   {"ttimu","ttimu""repeatedly activates IMU_EV.\n\r", testTaskingIMU},
                   {"rgps","rgps""reads gps[] string.\n\r", readGPS},
                   {"ttx","ttx""outputs 01010101.\n\r", testTX},
                   {"led","Test LED driver, if argc>0 [char] will convert char to int and drive that pattern.\n\r", testLED_cmd},
                   {"stime","start TimerA0, which calls bno055_get_IMU()", start_timer},

                   //ARC_COMMANDS,CTL_COMMANDS,ERROR_COMMANDS, // add lib functions to the help list 
                   //end of list
                   {NULL,NULL,NULL}};//
