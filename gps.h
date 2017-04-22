#ifndef __GPS_H
#define __GPS_H

  void gps_setup(void);
  double gps_mintodeg(double);
//  float gps_sectodeg(float);
  void gps_RX(void);
//  double gps_time(void);
  double gps_lat(void);
  double gps_lon(void);
  double gps_alt(void);
  double gps_latd(void);
  double gps_latm(void);
  double gps_lond(void);
  double gps_lonm(void);
  extern char gps[100];
#endif
