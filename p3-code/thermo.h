#ifndef THERMO_H
#define THERMO_H 1

#include <stdio.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// thermostat structs/functions

// Breaks temperature down into constituent parts
typedef struct{
  short tenths_degrees;         // actual temp in tenths of degrees
  char temp_mode;               // 1 for celsius, 2 for fahrenheit, 3 for error
} temp_t;

// Functions to implement for thermometer problem
int set_temp_from_ports(temp_t *temp);
int set_display_from_temp(temp_t temp, int *display);
int thermo_update();

////////////////////////////////////////////////////////////////////////////////
// thermo_sim.c structs/functions; do not modify

extern short THERMO_SENSOR_PORT;
// Set by the sensor to indicate temperature. Value is a positive int
// in units of 0.1 / 32 deg C above -45.0 deg C with a max of +45.0
// deg C. Above the max, the sensor becomes unreliable and below 0
// indicates the senor is failing.

extern unsigned char THERMO_STATUS_PORT;
// Bit 5 indicates whether display should be in Celsius (0) or
// Fahrenheit (1). This bit is toggled using a button on the
// thermometer but is set using command line args in the simulator.
// 
// Bit 2 is 0 for normal operation or 1 for an ERROR state. When in
// ERROR state, ERR should be shown on the display.
//
// Remaining bits may be 0 or 1 and should be ignored as they are not
// relevant to the display or temperature sensor.

extern int THERMO_DISPLAY_PORT;
// Controls thermometer display. Readable and writable. Routines
// wishing to change the display should alter the bits of this
// variable.

////////////////////////////////////////////////////////////////////////////////
// data and printing routines for displaying bits nicely
typedef struct {
  int nbits;
  int nclusters;
  int clusters[32];
} bitspec_t;

extern bitspec_t dispspec;
extern bitspec_t statspec;

char *bitstr(int x, bitspec_t *spec);
char *bitstr_index(bitspec_t *spec);

void print_thermo_display();
// Show the thermometer display as ACII on the screen

#endif
