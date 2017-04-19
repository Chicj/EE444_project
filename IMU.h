#ifndef __IMU_H
#define __IMU_H

extern unsigned short addr;  // set global address for now 
extern unsigned char glb_buff[10];  // global buffer
extern unsigned char eul_buff[8];  // global buffer 

extern const char *opr_mode_strings[];
extern const char *pwr_mode_strings[];
extern const char *sys_status_strings[];
extern const char *sys_err_strings[];

void  initIMUtimer(void);
short bno055_pageid(unsigned char page);
short bno055_reset(void);
short bno055_status(void);
short bno055_get_oprmode(void);
short bno055_set_oprmode_default(void);
short bno055_set_oprmode(unsigned char op_mode);
short bno055_get_quat(void);
short bno055_get_euler(void);
short bno055_get_imu(void);
short bno055_get_pwrmode(unsigned char op_mode);



#endif

