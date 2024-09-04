#include "thermo.h"

int set_temp_from_ports(temp_t *t){
    if (THERMO_SENSOR_PORT < 0 || THERMO_SENSOR_PORT > 28800 ) {
        t->tenths_degrees = 0;
        t->temp_mode = 3;
        return 1;
    }
    if(THERMO_STATUS_PORT & (1 << 2)){
        t->tenths_degrees = 0;
        t->temp_mode = 3;
        return 1;
    }
    int sensor_value = THERMO_SENSOR_PORT;
    int tenths_degrees_celsius = (sensor_value >> 5) - 450;
    if ((sensor_value & 0x1F) >= 16) {
        tenths_degrees_celsius++;
    }
    if(THERMO_STATUS_PORT & (1 << 5)) {
        t->tenths_degrees = (tenths_degrees_celsius *9)/5 + 320;
        t->temp_mode = 1;
    }
    return 0;
}

int set_display_from_temp(temp_t temp, int *display){
  if(temp.temp_mode ==1){
    if(temp.tenths_degrees < -450 || temp.tenths_degrees > 450){
      *display = 0b00000110111101111110111110000000;
      return 1;
    }
    else if(temp.temp_mode == 2){
      if (temp.tenths_degrees < -490 || temp.tenths_degrees > 1130){
        *display = 0b00000110111101111110111110000000;
        return 1;
      }
    }
    if(temp.temp_mode != 1 && temp.temp_mode != 2){
      *display = 0b00000110111101111110111110000000;
      return 1;
    }
    int bits[10] = {
      0b1111011, 0b1001000, 0b0111101, 0b1101101, 0b1001110, 0b1100111, 0b1110111, 0b1001001, 0b1111111, 0b1101111
    };
    int negative = 0;
    int t = temp.tenths_degrees;

    if (t < 0){
      negative = 1;
      t = -t;
    }
    int hundreds = t/1000;
    int tens = (t / 100) % 10;
    int ones = (t/10) % 10 ;
    int tenths = t % 10;

    *display = 0;
    if(hundreds > 0){
      *display |= bits[hundreds] << 21;
    }
    if(hundreds > 0 || tens > 0){
      *display |= bits[tens] << 14;
    }
    *display |=bits[ones] << 7;
    *display |=bits[tenths];

    if(negative){
      if(tens>0){
        *display |= 0b0000100 << 21;
      }
      else if(ones>0){
        *display |= 0b0000100 << 14;
      }
      else if(tenths>0){
        *display |= 0b0000100 << 7;
      }
    }
    if(temp.temp_mode == 1){
      *display |= 1  << 28;
    }else if (temp.temp_mode == 2){
      *display |= 1 << 29;
    }
    return 0;
  }
  return 1;
}

int thermo_update(){
    temp_t temp;
    int display;
    int result_temp = set_temp_from_ports(&temp);
    int result_display = set_display_from_temp(temp, &display);

    THERMO_DISPLAY_PORT = display;
    return result_temp == 0 && result_display == 0 ? 0 : 1;
}
