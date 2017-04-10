#ifndef __IMU_H
#define __IMU_H

//extern unsigned short addr = BNO055_I2C_ADDR1;  // set global address for now 
extern unsigned char glb_buff[10];  // global buffer 

short bno055_pageid(unsigned short addr,unsigned char *page);
short bno055_status(void);

//enum 

#endif

