#ifndef __GPS_H
#define __GPS_H

  void gps_setup(void);
  float gps_mintodeg(float);
  float gps_sectodeg(float);
  void gps_RX(void);

  extern char gps[100];
#endif
