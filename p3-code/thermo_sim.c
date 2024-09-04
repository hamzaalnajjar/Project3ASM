// thermo_sim.c: DO NOT MODIFY
//
// Thermometer simulator support functions.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "thermo.h"

#define THERMO_MAX_BITS 30


int THERMO_DISPLAY_PORT = 0;
// Global variable used to control the THERMO display. Making changes to
// this variable will change the thermostat display. Type ensures 32 bits.

short THERMO_SENSOR_PORT = 0;
// Set by the sensor to indicate temperature. Value is a positive int
// in units of 0.1 / 32 deg C above -45.0 deg C with a max of +45.0
// deg C. Above the max, the sensor becomes unreliable and below 0
// indicates the senor is failing.
// 
// Temperature above -45 C in 0.1/32 degrees C increments.
// 
//      0 is -45.0 C
//     32 is -44.9 C
//     64 is -44.8 C
//  14400 is   0.0 C
//  14432 is   0.1 C
//  24000 is  30.0 C
//  28800 is  45.0 C
// 
// If negative, sensor is erroring and display should show an
// error. In a real machine this would be set by physical hardware and
// read only.

unsigned char THERMO_STATUS_PORT = 0;
// Bit 2 is 0 for normal operation or 1 for an ERROR state. When in
// ERROR state, ERR should be shown on the display.
// 
// Bit 5 indicates whether display should be in Celsius (0) or
// Fahrenheit (1). This bit is toggled using a button on the
// thermometer but is set using command line args in the simulator.


////////////////////////////////////////////////////////////////////////////////
// Data and functions to set the state of the thermo clock display 

#define NROWS 5
#define NCOLS 23
// Convenience struct for representing the thermometer display  as characters 
typedef struct {
  char chars[NROWS][NCOLS];
} thermo_display;

//      ~~   ~~   ~~      0
//     |    |  | |  |     1
// ~~   ~~   ~~       O   2
//        | |  | |  |  F  3
//      ~~   ~~ o ~~      4
//012345678901234567890123
//0         1         2   

// Initial display config with decimal place
thermo_display init_display = {
  .chars = {
    "                     ",
    "                     ",
    "                     ",
    "                     ",
    "              o      ",
  }
};

// Reset thermo_display to be empty 
void reset_thermo_display(thermo_display *disp){
  *disp = init_display;
}

// Print an thermo_display 
void internal_print_thermo_display(thermo_display *thermo){
  for(int i=0; i<NROWS; i++){
    printf("%s\n",thermo->chars[i]);
  }
}  

// Position and char in display 
typedef struct {
  int r,c; int ch;
} charpos;
    
// Collection of characters corresponding to one bit in the state being set 
typedef struct {
  int len;                      // number of chars corresponding to this bit
  charpos pos[2];               // position of chars for this bit
} charpos_coll;


#define TOP 0
#define LFT 0
#define LMD 5
#define RMD 10
#define RGT 15


// Correspondence of bit positions to which characters should be set 
charpos_coll bits2chars[THERMO_MAX_BITS] = {
  { .len=2, .pos={{0+TOP,1+RGT,'~'}, {0+TOP,2+RGT,'~'}}}, //  0
  { .len=1, .pos={{1+TOP,0+RGT,'|'},                  }}, //  1
  { .len=2, .pos={{2+TOP,1+RGT,'~'}, {2+TOP,2+RGT,'~'}}}, //  2
  { .len=1, .pos={{1+TOP,3+RGT,'|'},                  }}, //  3
  { .len=1, .pos={{3+TOP,0+RGT,'|'},                  }}, //  4
  { .len=2, .pos={{4+TOP,1+RGT,'~'}, {4+TOP,2+RGT,'~'}}}, //  5
  { .len=1, .pos={{3+TOP,3+RGT,'|'},                  }}, //  6

  { .len=2, .pos={{0+TOP,1+RMD,'~'}, {0+TOP,2+RMD,'~'}}}, //  7
  { .len=1, .pos={{1+TOP,0+RMD,'|'},                  }}, //  8
  { .len=2, .pos={{2+TOP,1+RMD,'~'}, {2+TOP,2+RMD,'~'}}}, //  9
  { .len=1, .pos={{1+TOP,3+RMD,'|'},                  }}, // 10
  { .len=1, .pos={{3+TOP,0+RMD,'|'},                  }}, // 11
  { .len=2, .pos={{4+TOP,1+RMD,'~'}, {4+TOP,2+RMD,'~'}}}, // 12
  { .len=1, .pos={{3+TOP,3+RMD,'|'},                  }}, // 13

  { .len=2, .pos={{0+TOP,1+LMD,'~'}, {0+TOP,2+LMD,'~'}}}, // 14
  { .len=1, .pos={{1+TOP,0+LMD,'|'},                  }}, // 15
  { .len=2, .pos={{2+TOP,1+LMD,'~'}, {2+TOP,2+LMD,'~'}}}, // 16
  { .len=1, .pos={{1+TOP,3+LMD,'|'},                  }}, // 17
  { .len=1, .pos={{3+TOP,0+LMD,'|'},                  }}, // 18
  { .len=2, .pos={{4+TOP,1+LMD,'~'}, {4+TOP,2+LMD,'~'}}}, // 19
  { .len=1, .pos={{3+TOP,3+LMD,'|'},                  }}, // 20

  { .len=2, .pos={{0+TOP,1+LFT,'~'}, {0+TOP,2+LFT,'~'}}}, // 21
  { .len=1, .pos={{1+TOP,0+LFT,'|'},                  }}, // 22
  { .len=2, .pos={{2+TOP,1+LFT,'~'}, {2+TOP,2+LFT,'~'}}}, // 23
  { .len=1, .pos={{1+TOP,3+LFT,'|'},                  }}, // 24
  { .len=1, .pos={{3+TOP,0+LFT,'|'},                  }}, // 25
  { .len=2, .pos={{4+TOP,1+LFT,'~'}, {4+TOP,2+LFT,'~'}}}, // 26
  { .len=1, .pos={{3+TOP,3+LFT,'|'},                  }}, // 27

  { .len=2, .pos={{ 1, 20,'o'}, { 2, 21,'C'},} }, // 28
  { .len=2, .pos={{ 2, 20,'o'}, { 3, 21,'F'},} }, // 29
  // { .len=2, .pos={{ 3, 23,L'°'}, { 3, 1,'C'},} }, // 28
  // { .len=2, .pos={{ 4, 23,L'°'}, { 4, 1,'F'},} }, // 29
};

// Assumes ints are at least 32 bits 
void set_thermo_display(thermo_display *thermo, int disp){
  int i,j;
  int mask = 0x1;
  reset_thermo_display(thermo);
  for(i=0; i<THERMO_MAX_BITS; i++){
    //    printf("Checking %d\n",i);
    if( disp & (mask << i) ){ // ith bit set, fill in characters 
      //      printf("%d IS SET\n",i);
      charpos_coll coll = bits2chars[i];
      //      printf("Coll len: %d\n",coll.len);
      for(j=0; j<coll.len; j++){
        //        printf("Inner iter %d\n",j);
        charpos pos = coll.pos[j];
        thermo->chars[pos.r][pos.c] = pos.ch;
        // print_thermo_display(thermo);
      }
    }
  }
}


// Use the global THERMO_DISPLAY_PORT to print the time 
void print_thermo_display(){
  thermo_display thermo;
  set_thermo_display(&thermo, THERMO_DISPLAY_PORT);
  internal_print_thermo_display(&thermo);
  return;
}


bitspec_t dispspec = {
  .nbits = 32,
  .nclusters = 6,
  .clusters = {2, 2, 7, 7, 7, 7},
};

bitspec_t statspec = {
  .nbits = 8,
  .nclusters = 2,
  .clusters = {4,4},
};

// Generate a string version of the bits which clusters the bits based
// on the logical groupings in the problem
char *bitstr(int x, bitspec_t *spec){
  static char buffer[512];
  int idx = 0;
  int clust_idx = 0;
  int clust_break = spec->clusters[0];
  int max = spec->nbits-1;
  for(int i=0; i<spec->nbits; i++){
    if(i==clust_break){
      buffer[idx] = ' ';
      idx++;
      clust_idx++;
      clust_break += spec->clusters[clust_idx];
    }
    buffer[idx] = x & (1 << (max-i)) ? '1' : '0';
    idx++;
  }
  buffer[idx] = '\0';
  return buffer;
}

// Creates a string of bit indices matching the clustering pattern
// above
char *bitstr_index(bitspec_t *spec){
  static char buffer[512];
  char format[256];
  int pos = 0;
  int idx = spec->nbits;
  for(int i=0; i<spec->nclusters; i++){
    idx -= spec->clusters[i];
    if(spec->clusters[i] > 1){
      sprintf(format, "%s%dd ","%",spec->clusters[i]);
      pos += sprintf(buffer+pos, format, idx);
    }
    else{                       // cluster of 1 bit gets no index
      pos += sprintf(buffer+pos, "  ");
    }
  }
  buffer[pos-1] = '\0';         // eliminate trailing space
  return buffer;
}


// // Print the most signficant (right-most) to least signficant bit in
// // the integer passed in 
// void showbits(int x, int bits){
//   int i;
//   int mask = 0x1;
//   for(i=bits-1; i>=0; i--){
//     int shifted_mask = mask << i;
//     if(shifted_mask & x){
//       putchar('1');
//     } else {
//       putchar('0');
//     }
//   }
//   // Equivalent short version 
//   //    (x&(1<<i)) ? putchar('1'): putchar('0');
// }

// char *asbits(int x, int bits){
//   static char buffer[256];
//   int i,idx=0;
//   int mask = 0x1;
//   for(i=bits-1; i>=0; i--, idx++){
//     int shifted_mask = mask << i;
//     buffer[idx]= (shifted_mask & x) ? '1' : '0';
//   }
//   buffer[idx] = '\0';
//   return buffer;
// }

