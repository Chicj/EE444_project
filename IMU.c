#include <bno055.h>
#include "i2c.h"

//TODO list
/* create a set addr function 
*\

//TODO test this 
/* Changes accessible IMU register page */
short bno055_pageid(unsigned short addr,unsigned char *page){
  unsigned char tx_buf[1];
  short resp;

  resp = i2c_tx(addr, page, 1);//set pageid
  return resp;
}





