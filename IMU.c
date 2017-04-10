#include <bno055.h>
#include "i2c.h"
#include "IMU.h"

unsigned short addr = BNO055_I2C_ADDR1; // global IMU addr
unsigned char glb_buff[10];  // global buffer 
char *configmode[13] = {"CONFIGMODE", "ACCONLY", "MAGONLY", "GYROONLY", "ACCMAG", "ACCGYRO", "MAGGYRO", "AMG", "IMU", "COMPASS", "M4G", "NDOF_FMC_OFF", "NDOF"};
//TODO list
/*
 create a set addr function ? only needed for multi IMU's
 create a read sys_err function 
*\

//TODO test this 
/* Changes accessible IMU register page */
short bno055_pageid(unsigned short addr,unsigned char *page){
  unsigned char tx_buf[1];
  short resp;

  resp = i2c_tx(addr, page, 1);//set pageid
  return resp;
}

/* This command will check the status of the IMU board reads 0x35 --> 0x38*/
short bno055_status(void){
  unsigned char tx_buf[1] = {BNO055_CALIB_STAT_ADDR};
  short resp;
    
  resp = i2c_txrx(addr ,tx_buf ,1 ,glb_buff ,4);// read status data
  return resp;
}

// set opp mode 
short bno055_oprmode(unsigned char *op_mode){
  unsigned char tx_buf[2] = {BNO055_OPR_MODE_ADDR};
  unsigned short resp;

  resp = i2c_txrx(addr, tx_buf, 1, glb_buff, 2); // read opr_mode 
    i2c_tx(addr, tx_buf, 1);// write opp mode
  return resp;
}

//read IMU system error codes
short bno55_syserr(void){
  unsigned char tx_buf[1] = {BNO055_SYS_ERR_ADDR};
  unsigned short resp;
  resp = i2c_txrx(addr ,tx_buf ,1 ,glb_buff ,2);// read sys_err reg
  return resp;
}



//TODO set power mode 
short bno055_power(unsigned char op_mode){
  unsigned char tx_buf[1] = {BNO055_PWR_MODE_ADDR};
  i2c_txrx(addr, tx_buf, 1, glb_buff, 2); // read opr_mode 
  return 0;
}

