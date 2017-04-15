/*
REFERENCES

Longitude and latitude stuff: point-to-point distance, bearing, midpoint, destination, intersection, cross-track distance, closest point to poles, rhumb lines
http://www.movable-type.co.uk/scripts/latlong.html
*/

#include <msp430.h> 
#include <math.h>
#include <stdio.h>
#include <pathfinding.h>
#include <ctl.h>

float apos[3] = {0.0,0.0,0.0};    // astronaut combined position from GPS and IMU
float aposGPS[3] = {0.0,0.0,0.0}; // astronaut global position from GPS [latitude (degrees), longitude (degrees), altitude (meters)]
float aposIMU[3] = {0.0,0.0,0.0}; // astronaut relative position from IMU [x (meters), y (meters), z (meters)] relative to last GPS ping
float ahed = 0.0;                 // astronaut heading relative to north (in radians)
float arot = 0.0;                 // astronaut rotation needed to be facing target (in degrees for now)

int tWP = 0;                    // current target waypoint
float tpos[3] = {0.0,0.0,0.0};  // coordinates of current target waypoint
float thed = 0.0;               // heading needed for astronaut to be facing target

CTL_EVENT_SET_t PF_events;

void PF_func(void *p) __toplevel{
  unsigned int e;
  for (;;) {
    e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR, &PF_events, PF_EV_ALL, CTL_TIMEOUT_NONE, 0);
    // GPU STUFF HAPPENED
    if (e & GPU_EV){

    }
    // IMU STUFF HAPPENED
    if (e & IMU_EV){

    }
  }
}


//float debugWP[5][3] = { // test waypoints with coordinates
//  {64.856272,-147.819532,177.4}, // Flag Circle
//  {64.857242,-147.821270,177.4}, // Turtle Sex Park
//  {64.857065,-147.823148,177.4}, // The Lonely Tree
//  {64.856471,-147.822139,177.4}, // Gruelson Junction
//  {64.855865,-147.821099,177.4}  // Signer's Bus Bench
//}; // elevation of 177.4 may not be reachable at all locations, taken at flag circle.

float debugWP[6][3] = { // test waypoints with coordinates
  {10.0,10.0,0.0},
  {10.0,20.0,0.0},
  {-10.0,-10.0,0.0},
  {-20.0,20.0,0.0},
  {-50.0,50.0,0.0},
  {50.0,-40.0,0.0}
  };


// Initializes everything necessary for pathfinding algorithm to work.
void initPathfinding (void)
{
  tpos[0] = debugWP[0][0];
  tpos[1] = debugWP[0][1];
  tpos[2] = debugWP[0][2];
}


// Takes latitude, longitude, and altitude from GPS and applies it to global position variable.
void pathfindGPS (float templat, float templon, float tempalt)
{
  // remember where the last GPS ping was
  aposGPS[0] = templat; // latGPS
  aposGPS[1] = templon; // lonGPS
  aposGPS[2] = tempalt; // altGPS

  // set current position to GPS ping
  apos[0] = aposGPS[0]; // alat
  apos[1] = aposGPS[1]; // alon
  apos[2] = aposGPS[2]; // aalt
}


// Takes relative x, y, and z distances from IMU and applies it to relative position variable.
void pathfindIMU (/*float dx, float dy, float dz,*/ float dhed)
{
//  // update the relative position from IMU.
//  aposIMU[0] += dx; // latIMU
//  aposIMU[1] += dy; // lonIMU
//  aposIMU[2] += dz; // altIMU

  // update astronaut heading from IMU
  ahed = dhed * M_PI / 180; // hedIMU
}


//// Combines last GPS global location with IMU current relative position to calculate current global location.
//void pathfindPosition (void)
//{
//  // find current altitude, for use as current planet radius in current latitude and longitude calculations.
//  apos[2] = aposGPS[2] + aposIMU[2]; // alt or Radius = altGPS + altIMU
//
//  // calculate current latitude and longitude
//  // ###!### NEED TO MAKE SURE latIMU AND lonIMU ARE RELATED TO NORTH AND EAST HEADINGS ###!###
//  apos[0] = aposGPS[0] + ((aposIMU[0] / apos[2]) * 180 / M_PI); // lat = latGPS + latIMU = latGPS + (dNorth / Radius)
//  apos[1] = aposGPS[1] + ((aposIMU[1] / (apos[2] * cos(M_PI * aposGPS[0] / 180))) * 180 / M_PI); // lon = lonGPS + lonIMU = lonGPS + [dEast  / (Radius * cos(pi * latGPS / 180))]
//
////  // if the astronaut crosses the 180/-180 degree longitude line, loop the coordinates.
////  if (apos[1] > 180) { apos[1] -= 360; }
////  else if (apos[1] < -180) { apos[1] += 360; }
//}


// determine appropriate target waypoint and determine heading to target waypoint
void pathfindTarget (void)
{
  // if within range of target waypoint, set next waypoint on path as target
  if (pathfindDistance() <= WP_PROX)
  {
    tWP++; // may overflow!
    tpos[0] = debugWP[tWP][0]; // tlat
    tpos[1] = debugWP[tWP][1]; // tlon
    tpos[2] = debugWP[tWP][2]; // talt
  }
}


// determine distance between astronaut and target waypoint
float pathfindDistance (void)
{
  float tempalat = apos[0] * M_PI / 180;
  float temptlat = tpos[0] * M_PI / 180;
  float dlat = (tpos[0] - apos[0]) * M_PI / 180;
  float dlon = (tpos[1] - apos[1]) * M_PI / 180;
  float a = pow(sin(dlat / 2), 2) + (cos(tempalat) * cos(temptlat) * pow(sin(dlon / 2), 2));
  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  float d = MSL_EARTH * c; // consider working altitude into this equation.

  return d;
}


// determine astronaut heading to target waypoint
void pathfindHeading (void)
{
  float tempalat = apos[0] * M_PI / 180;
  float temptlat = tpos[0] * M_PI / 180;
  float dlat = (tpos[0] - apos[0]) * M_PI / 180;
  float dlon = (tpos[1] - apos[1]) * M_PI / 180;
  //thed = atan2((tpos[1] - apos[1]), (tpos[0] - apos[0])); // angle from north to target (in radians)
  thed = atan2(sin(dlon) * cos(temptlat), cos(tempalat) * sin(temptlat) - sin(tempalat) * cos(temptlat) * cos(dlon)); // angle from north to target (in radians)
}

// determine astronaut rotation needed to be facing target waypoint
float pathfindPoint (void)
{
  arot = (thed - ahed) * 180 / M_PI;
  return arot; // degrees astronaut needs to rotate (in degrees)
  // may go over +-180 degrees!
}
