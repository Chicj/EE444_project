#include <bno055.h>
#include <msp430f6779a.h>
#include "i2c.h"
#include "IMU.h"
#include <ctl.h>
#include "pathfinding.h"
#include <stdio.h>

unsigned short addr = BNO055_I2C_ADDR1; // global IMU addr
unsigned char glb_buff[10];  // global buffer
unsigned char eul_buff[8];
const char *opr_mode_strings[] = {"CONFIGMODE", "ACCONLY", "MAGONLY", "GYROONLY", "ACCMAG", "ACCGYRO", "MAGGYRO", "AMG", "IMU", "COMPASS", "M4G", "NDOF_FMC_OFF", "NDOF"};
const char *pwr_mode_strings[] = {"Normal Mode", "Low Power Mode", "Suspend Mode"};
const char *sys_status_strings[] = {"System Idle", "System Error", "Initializing Peripherals", "System Initialization", "Executing Selftest", "Sensor Fusion Algorithm Running", "System Running Without Fusion Algorithm"};
const char *sys_err_strings[] = {"No Error", "Peripheral Initialization Error", "System Initialization Error", "Self Test Result Error", "Register Map Value Out of Range", "Register Map Address out of Range", "Register Map Write Error", "BNO Low Power Mode Not Available for Selected Operation Mode", "Accelerometer Power Mode Not Available", "Fusion Algorithm Configuration Error", "Sensor Configuration Error"};

//TODO list
/*
 create a set addr function ? only needed for multi IMU's
 create a read sys_err function 
*/


void initIMUtimer(void) {
  // TODO get timer to interrupt without doing something weird in assembly... still interrupts at TA0CCR0 speed.
  // TODO clear IFG at end of interrupt?

  // CLK = ACLK / ID / IDEX = 32768 / 0 / 0 = 32768 Hz
  // N = T * CLK / (1 + UPDOWN?) = 0.100 * 32768 / (1 + 0) = 3276.8
  // trigger every 100 ms, 10 Hz
  TA0CCR0 = 65535; //3277; 
  //TA0CTL = TACLR;
  TA0CTL = TASSEL__ACLK + MC__STOP + TAIE;
  //TA0CTL |= MC__UP;
}


void initIMU(void) {
  short resp, gyr_calib;
  long i;
  unsigned char tx_buf[2]={BNO055_SYS_TRIGGER_ADDR, BNO055_SYS_RST_MSK};
  //for (i = 0; i < 100000; i++);
  //resp = bno055_reset();
  
  /*while (resp < 0) {
    printf("IMU init: Failed to set  operation mode.\n\r");
    //__delay_cycles(100000);
    resp = i2c_tx(addr, tx_buf, 2);
  }*/
  //for (i = 0; i < 100000; i++);
  resp = bno055_set_oprmode_default();
  

  //gyr_calib = (glb_buff[0] & (BIT4+BIT5))>>4;
  /*while (gyr_calib == 0) {
    printf("IMU init: Not calibrated yet.\n\r");
    //__delay_cycles(100000);
    i2c_txrx(addr, tx_buf, 1, glb_buff, 2);
    gyr_calib = (glb_buff[0] & (BIT4+BIT5))>>4;
  }*/
}


// Reset IMU
short bno055_reset(void)
{
  unsigned char tx_buf[2]={BNO055_SYS_TRIGGER_ADDR, BNO055_SYS_RST_MSK};
  return i2c_tx(addr, tx_buf, 2);
}


//TODO test this 
/* Changes accessible IMU register page */
short bno055_pageid(unsigned char page){
  unsigned char tx_buf[1];
  tx_buf[0] = page;
  return i2c_tx(addr, tx_buf, 2);//set pageid
}


// This command will check the status of the IMU board reads 0x35 --> 0x39
short bno055_status(void)
{
  unsigned char tx_buf[1] = {BNO055_CALIB_STAT_ADDR};  
  return i2c_txrx(addr, tx_buf, 1, glb_buff, 6);// read status data
}


// Get IMU Operating Mode
short bno055_get_oprmode(void)
{
  unsigned short resp;
  unsigned char tx_buf[2] = {BNO055_OPR_MODE_ADDR};
  resp = i2c_txrx(addr, tx_buf, 1, glb_buff, 2); // read opr_mode
  glb_buff[0] &= BNO055_OPERATION_MODE_MSK;
  glb_buff[1] &= BNO055_POWER_MODE_MSK;
  return resp;
}


// Set IMU Operating Mode to IMU (Fusion mode with Gyro+Accel)
short bno055_set_oprmode_default(void)
{
  unsigned char tx_buf[2] = {BNO055_OPR_MODE_ADDR, BNO055_OPERATION_MODE_IMUPLUS};
  return i2c_tx(addr, tx_buf, 2);// write opp mode
}


// Set IMU Operating Mode to IMU (Fusion mode with Gyro+Accel)
short bno055_set_oprmode(unsigned char op_mode){
  unsigned char tx_buf[2] = {BNO055_OPR_MODE_ADDR};
  tx_buf[1] = op_mode;
  return i2c_tx(addr, tx_buf, 2);// write opp mode
}


// Get IMU system error codes
short bno55_syserr(void)
{
  unsigned char tx_buf[1] = {BNO055_SYS_ERR_ADDR};
  return i2c_txrx(addr, tx_buf, 1, glb_buff, 2);// read sys_err reg
}


// Get IMU Quaternion data
short bno055_get_quat(void)
{
  unsigned char tx_buf[1] = {BNO055_QUATERNION_DATA_W_LSB_ADDR};
  return i2c_txrx(addr, tx_buf, 1, glb_buff, 8);// read sys_err reg
}


// Get IMU Euler data
short bno055_get_euler(void)
{
  unsigned short resp;
  unsigned char tx_buf[1] = {BNO055_EULER_H_LSB_ADDR};
  resp = i2c_txrx(addr, tx_buf, 1, eul_buff, 8);// read sys_err reg
  return resp;
}


// Get IMU Euler data and start IMU task
short bno055_get_imu(void)
{
  short imu_stat;
  long int i;
  unsigned short resp;
  unsigned char tx_buf[1] = {BNO055_EULER_H_LSB_ADDR};
  //for (i = 0; i < 100000; i++);
  // TODO with timer interrupt, bno055_get_imu() never makes it back from 12c_txrx()
  imu_stat = bno055_errcheck();
  resp = i2c_txrx(addr, tx_buf, 1, eul_buff, 8); // read sys_err reg
  ctl_events_set_clear(&PF_events, IMU_EV, 0); // will be used in bno055_get_IMU instead...
  ctl_events_set_clear(&PF_events, LED_EV, 0);  // Drive LED
  return resp;
}


// Get IMU power mode
//TODO set power mode 
short bno055_get_pwrmode(unsigned char op_mode){
  unsigned char tx_buf[1] = {BNO055_PWR_MODE_ADDR};
  return i2c_txrx(addr, tx_buf, 1, glb_buff, 2);
}


short bno055_errcheck(void){
  unsigned char tx_buf[1] = {BNO055_SYS_STAT_ADDR};  
  i2c_txrx(addr, tx_buf, 1, glb_buff, 2);// read sys_stat data and sys_er

  if(glb_buff[0] == 1){
    bno055_reset(); // reset IMU if in err state
    return 1; // err found 
  }
  return 0; // no err
}
