#ifndef __IMU_H
#define __IMU_H

extern unsigned short addr = 0x28; // BNO055_i2c_address 


short bno055_pageid(unsigned short addr,unsigned char *page);


#endif

