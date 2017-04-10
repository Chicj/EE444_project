#ifndef __IMU_H
#define __IMU_H

extern unsigned char glb_buff[10];  // global buffer 

short bno055_pageid(unsigned short addr,unsigned char *page);
short bno055_status(void);


#endif

