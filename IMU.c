#include <bno055.h>
#include "i2c.h"
#include "IMU.h"

unsigned short addr = BNO055_I2C_ADDR1; // global IMU addr
unsigned char glb_buff[10];  // global buffer 
const char *opr_mode_strings[] = {"CONFIGMODE", "ACCONLY", "MAGONLY", "GYROONLY", "ACCMAG", "ACCGYRO", "MAGGYRO", "AMG", "IMU", "COMPASS", "M4G", "NDOF_FMC_OFF", "NDOF"};
const char *pwr_mode_strings[] = {"Normal Mode", "Low Power Mode", "Suspend Mode"};
const char *sys_status_strings[] = {"System Idle", "System Error", "Initializing Peripherals", "System Initialization", "Executing Selftest", "Sensor Fusion Algorithm Running", "System Running Without Fusion Algorithm"};
const char *sys_err_strings[] = {"No Error", "Peripheral Initialization Error", "System Initialization Error", "Self Test Result Error", "Register Map Value Out of Range", "Register Map Address out of Range", "Register Map Write Error", "BNO Low Power Mode Not Available for Selected Operation Mode", "Accelerometer Power Mode Not Available", "Fusion Algorithm Configuration Error", "Sensor Configuration Error"};

//TODO list
/*
 create a set addr function ? only needed for multi IMU's
 create a read sys_err function 
*/

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
  return i2c_txrx(addr, tx_buf, 1, glb_buff, 5);// read status data
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

// Get IMU power mode
//TODO set power mode 
short bno055_get_pwrmode(unsigned char op_mode){
  unsigned char tx_buf[1] = {BNO055_PWR_MODE_ADDR};
  return i2c_txrx(addr, tx_buf, 1, glb_buff, 2);
}

