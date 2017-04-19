#ifndef __PATHFINDING_H
#define __PATHFINDING_H

// Everything is global for now, for debugging.
// Nothing but pathfinding.c and commands.c should use these variables.
#include <ctl.h>

extern double apos[3];     // astronaut combined position from GPS and IMU
//extern float aposGPS[3];  // astronaut global position from GPS [latitude (degrees), longitude (degrees), altitude (meters)]
//extern float aposIMU[3];  // astronaut relative position from IMU [x (meters), y (meters), z (meters)] relative to last GPS ping
extern float ahed;        // astronaut heading relative to north (in radians)
extern float arot;        // astronaut rotation needed to be facing target (in degrees for now)

extern int tWP;       // current target waypoint
extern float tpos[3]; // coordinates of current target waypoint
extern float thed;    // heading needed for astronaut to be facing target

//extern float debugWP[6][3]; // test waypoints with coordinates

// GET RID OF THISSSSSSSSSSSSSSSSS
//#define M_PI (3)

// ********** TASKING STUFF **********
extern CTL_EVENT_SET_t PF_events;
enum{GPS_EV=(1<<0), IMU_EV=(1<<1),LED_EV=(1<<2)};
#define PF_EV_ALL (GPS_EV|IMU_EV|LED_EV)

void PF_func(void *p);
// ***********************************

void initPathfinding (void);
void pathfindGPS (float templat, float templon, float tempalt);
void pathfindIMU (float temphed);
void pathfindPosition(void);
void pathfindTarget (void);
float pathfindDistance (void);
void pathfindHeading (void);
float pathfindPoint (void);

#define MSL_EARTH 6371008.0 // Earth volumetric mean radius (Mean Sea Level) 6371.008 [km]
//#define MSL_MARS 3389500 // Mars volumetric mean radius (Mean Sea Level) 3389.500 [km]
#define WP_PROX 50.0 // proximity (in meters) the astronaut has to be within to proceed to next waypoint

#endif
