#include "thermo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PRINT_TEST sprintf(sysbuf,"awk 'NR==(%d+1){P=1;print \"{\"} P==1 && /ENDTEST/{P=0; print \"}\\nTEST OUTPUT:\"} P==1{print}' %s", __LINE__, __FILE__); \
  system(sysbuf);

void print_temp(temp_t *temp){
  printf("temp = {\n"); 
  printf("  .tenths_degrees = %d,\n",temp->tenths_degrees);
  printf("  .temp_mode      = %d,\n",temp->temp_mode);
  printf("}\n");
}

void print_ports(){
  printf("%-19s : %hd\n","THERMO_SENSOR_PORT", THERMO_SENSOR_PORT);
  printf("%-19s : %s\n", "THERMO_STATUS_PORT",  bitstr(THERMO_STATUS_PORT,&statspec));
  printf("%-19s : %s\n", "index",bitstr_index(&statspec));
  printf("%-19s : %s\n", "THERMO_DISPLAY_PORT",bitstr(THERMO_DISPLAY_PORT,&dispspec));
  printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
}

char *test_name;
int run_test(char *this_test){        // indicates whether a named test should be run
  if(strcmp(test_name, "all")==0){
    return 1;
  }
  if(strcmp(test_name, this_test)==0){
    return 1;
  }
  if(strcmp(test_name, "printall")==0){
    printf("%s\n",this_test);
    return 0;
  }
  return 0;
}


// defined in assembly to set up registers before calling required functions
int CALL_set_temp_from_ports(temp_t *);
int CALL_set_display_from_temp(temp_t, int *);
int CALL_thermo_update();

// Used in assembly wrapper to retrieve callee save registers
long callee_reg_exp = 0xBADBADBADBADBADD;
long callee_reg_vals[6] = {};
char *callee_reg_name[6] = {"%rbp", "%rbx", "%r12", "%r13", "%r14", "%r15"};

void check_callee_registers(){
  for(int i=0; i<6; i++){
    if(callee_reg_vals[i] != callee_reg_exp){
      char *regname = callee_reg_name[i];
      printf("%4s: Callee Register changed during function\n",regname);
      printf("      Did you use it and fail to restore it? Try a push/pop combination\n");
    }
  }
}

int main(int argc, char *argv[]){
  if(argc < 2){
    printf("usage: %s <test_name>\n", argv[0]);
    return 1;
  }
  test_name = argv[1];

  char sysbuf[4096];

  int *dispint = malloc(sizeof(int));      // used for tests that set the display
  temp_t *temp = malloc(sizeof(temp_t));   // used for tests that set the display
  int ret;

  if(0){}

  if( run_test("set_temp_from_ports() 0 C") ) {
    PRINT_TEST;
    // Tests whether sensor value of 0 with all 0 status
    // port is handled correctly.
    THERMO_SENSOR_PORT  = 0;
    THERMO_STATUS_PORT  = 0b00000000; // bit 5 is 0, Celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test("CALL_set_temp_from_ports() 0 F") ) {
    PRINT_TEST;
    // Tests whether sensor value of 0 with all 0 status
    // port is handled correctly.
    THERMO_SENSOR_PORT  = 0;
    THERMO_STATUS_PORT  = 0b00100000; // bit 5 is 1, Fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() 128 C/F" ) ) {
    PRINT_TEST;
    // Tests whether sensor value of 128 is handled
    // correctly for Celsius and Fahrenheit.
    printf("==CELSIUS==\n");
    THERMO_SENSOR_PORT  = 128;
    THERMO_STATUS_PORT  = 0b00000000; // bit 5 is 0, Celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    printf("==FAHRENHEIT==\n");
    THERMO_STATUS_PORT  = 0b00100000; // bit 5 is 1, Fahreheit
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() freezing C" ) ) {
    PRINT_TEST;
    // Tests whether sensor value of leads to 0 deg C.
    THERMO_SENSOR_PORT  = 14400;
    THERMO_STATUS_PORT  = 0b00000000; // bit 5 is 0, Celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() freezing F" ) ) {
    PRINT_TEST;
    // Tests whether sensor value of leads to 32 deg F.
    THERMO_SENSOR_PORT  = 14400;
    THERMO_STATUS_PORT  = 0b00100000; // bit 5 is 1, Fahreheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST


  if( run_test( "CALL_set_temp_from_ports() rounding C" ) ) {
    PRINT_TEST;
    THERMO_STATUS_PORT  = 0b00000000; // bit 5 is 0, Celsius
    THERMO_DISPLAY_PORT = -1;
    // Checks several rounding cases
    THERMO_SENSOR_PORT  = 47;         // rem 15, round down
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    check_callee_registers();
  } // ENDTEST
  if( run_test( "CALL_set_temp_from_ports() rounding C" ) ) {
    PRINT_TEST;
    // Checks several rounding cases
    THERMO_STATUS_PORT  = 0b00000000; // bit 5 is 0, Celsius
    THERMO_DISPLAY_PORT = -1;
    THERMO_SENSOR_PORT  = 48;         // rem 16, round up
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    check_callee_registers();
  } // ENDTEST
  if( run_test( "CALL_set_temp_from_ports() rounding C" ) ) {
    PRINT_TEST;
    // Checks several rounding cases
    THERMO_STATUS_PORT  = 0b00000000; // bit 5 is 0, Celsius
    THERMO_DISPLAY_PORT = -1;
    THERMO_SENSOR_PORT  = 90;         // rem 26, round up
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    check_callee_registers();
  } // ENDTEST


  if( run_test( "CALL_set_temp_from_ports() status nonzero" ) ) {
    PRINT_TEST;
    // Tests whether correct F/C is set when status has nonzeros
    THERMO_SENSOR_PORT  = 8000;
    THERMO_STATUS_PORT  = 0b11000001; // bit 5 is 0, Celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() status nonzero" ) ) {
    PRINT_TEST;
    // Tests whether correct F/C is set when status has nonzeros
    THERMO_SENSOR_PORT  = 8000;
    THERMO_STATUS_PORT  = 0b11100001; // bit 5 is 1, Fahreheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() sensor range" ) ) {
    PRINT_TEST;
    // Tests whether out of range sensor is correctly detected
    THERMO_SENSOR_PORT  = -200;
    THERMO_STATUS_PORT  = 0b00000000; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() sensor range" ) ) {
    PRINT_TEST;
    // Tests whether out of range sensor is correctly
    // detected and temp_status is set to 3 for error.
    THERMO_SENSOR_PORT  = 28805;
    THERMO_STATUS_PORT  = 0b00100000; // fahreheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() status error" ) ) {
    PRINT_TEST;
    // Tests whether bit 2 of the status port is checked;
    // when 1 the thermometer is erroring and temp_mode
    // should be set to 3.
    THERMO_SENSOR_PORT  = 600;
    THERMO_STATUS_PORT  = 0b10100100; // fahreheit+error
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() status error" ) ) {
    PRINT_TEST;
    // Tests whether bit 2 of the status port is checked;
    // when 1 the thermometer is erroring and temp_mode
    // should be set to 3.
    THERMO_SENSOR_PORT  = 600;
    THERMO_STATUS_PORT  = 0b01000101; // celsius+error
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() wide range" ) ) {
    PRINT_TEST;
    // Checks several temperatures in range for correct
    // calculation including maximal values. Status port
    // contains some non-zero values aside from c/f bit.
    THERMO_SENSOR_PORT  = 28800;      // max allowed
    THERMO_STATUS_PORT  = 0b11000001; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() wide range" ) ) {
    PRINT_TEST;
    // Checks several temperatures in range for correct
    // calculation including maximal values. Status port
    // contains some non-zero values aside from c/f bit.
    THERMO_SENSOR_PORT  = 28800;      // max allowed
    THERMO_STATUS_PORT  = 0b10110001; // fahreheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() wide range" ) ) {
    PRINT_TEST;
    // Checks several temperatures in range for correct
    // calculation including maximal values. Status port
    // contains some non-zero values aside from c/f bit.
    THERMO_SENSOR_PORT  = 27299;
    THERMO_STATUS_PORT  = 0b10110001; // fahreheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "CALL_set_temp_from_ports() wide range" ) ) {
    PRINT_TEST;
    // Checks several temperatures in range for correct
    // calculation including maximal values. Status port
    // contains some non-zero values aside from c/f bit.
    THERMO_SENSOR_PORT  = 27299;
    THERMO_STATUS_PORT  = 0b01010010; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    print_ports();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() 123 C" ) ) {
    PRINT_TEST;
    // Basic check to see if digit bits are set correctly
    // and would display properly.
    temp->tenths_degrees = 123;
    temp->temp_mode      = 1;         // celsius
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() 456 F" ) ) {
    PRINT_TEST;
    // Basic check to see if digit bits are set correctly
    // and would display properly.
    temp->tenths_degrees = 456;
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() 896 F" ) ) {
    PRINT_TEST;
    // Basic check to see if digit bits are set correctly
    // and would display properly.
    temp->tenths_degrees = 896;
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() 78 C" ) ) {
    PRINT_TEST;
    // Basic check to see if digit bits are set correctly
    // and would display properly.
    temp->tenths_degrees = 78;
    temp->temp_mode      = 1;         // celsius
    THERMO_SENSOR_PORT  = 128;        // ports should be ignored
    THERMO_STATUS_PORT  = 0b11111111;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() -90 F" ) ) {
    PRINT_TEST;
    // Checks if the negative sign aligns correctly to the
    // left middle digit for single digit temperatures.
    temp->tenths_degrees = -90;
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 128;        // ports should be ignored
    THERMO_STATUS_PORT  = 0b11111111;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() -234 C" ) ) {
    PRINT_TEST;
    // Checks that negative sign aligns correctly to the
    // left for 2-digit negative temps.
    temp->tenths_degrees = -234;
    temp->temp_mode      = 1;         // celsius
    THERMO_SENSOR_PORT  = 128;        // ports should be ignored
    THERMO_STATUS_PORT  = 0b11111111;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() above 100" ) ) {
    PRINT_TEST;
    // Checks that fahrenheit temps above 100 print correctly
    temp->tenths_degrees = 1000;
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() above 100" ) ) {
    PRINT_TEST;
    // Checks that fahrenheit temps above 100 print correctly
    temp->tenths_degrees = 1006;
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() above 100" ) ) {
    PRINT_TEST;
    // Checks that fahrenheit temps above 100 print correctly
    temp->tenths_degrees = 1037;
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() above 100" ) ) {
    PRINT_TEST;
    // Checks that fahrenheit temps above 100 print correctly
    temp->tenths_degrees = 1124;
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() extreme values C" ) ) {
    PRINT_TEST;
    // Checks that the extreme temps at the boundary of the
    // acceptable range are correctly printed.
    temp->tenths_degrees = -450;
    temp->temp_mode      = 1;         // celsius
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() extreme values C" ) ) {
    PRINT_TEST;
    // Checks that the extreme temps at the boundary of the
    // acceptable range are correctly printed.
    temp->tenths_degrees = 450;
    temp->temp_mode      = 1;         // celsius
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() extreme values F" ) ) {
    PRINT_TEST;
    // Checks that the extreme temps at the boundary of the
    // acceptable range are correctly printed.
    temp->tenths_degrees = -490;
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() extreme values F" ) ) {
    PRINT_TEST;
    // Checks that the extreme temps at the boundary of the
    // acceptable range are correctly printed.
    temp->tenths_degrees = 1130;
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST


  if( run_test( "set_display_from_temp() error range" ) ) {
    PRINT_TEST;
    // Checks that ERR is displayed if the temperature is
    // out of range in either celsius or fahrenheit.
    temp->tenths_degrees = -462;      // below min celsius
    temp->temp_mode      = 1;         // celsius
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() error range" ) ) {
    PRINT_TEST;
    // Checks that ERR is displayed if the temperature is
    // out of range in either celsius or fahrenheit.
    temp->tenths_degrees = -451;      // above max celsius
    temp->temp_mode      = 1;         // celsius
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() error range" ) ) {
    PRINT_TEST;
    // Checks that ERR is displayed if the temperature is
    // out of range in either celsius or fahrenheit.
    temp->tenths_degrees = -495;      // below min fahrenheit
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() error range" ) ) {
    PRINT_TEST;
    // Checks that ERR is displayed if the temperature is
    // out of range in either celsius or fahrenheit.
    temp->tenths_degrees = 1169;      // above max fahrenheit
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() error temp_mode" ) ) {
    PRINT_TEST;
    // Checks that ERR is displayed if the temp_mode field
    // is not set to celsius (1) or fahrenheit (2)
    temp->tenths_degrees = 250;       // ignored
    temp->temp_mode      = 3;         // error
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() error temp_mode" ) ) {
    PRINT_TEST;
    // Checks that ERR is displayed if the temp_mode field
    // is not set to celsius (1) or fahrenheit (2)
    temp->tenths_degrees = 320;       // ignored
    temp->temp_mode      = 8;         // error
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() error temp_mode" ) ) {
    PRINT_TEST;
    // Checks that ERR is displayed if the temp_mode field
    // is not set to celsius (1) or fahrenheit (2)
    temp->tenths_degrees = 17;        // ignored
    temp->temp_mode      = -1;        // error
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_display_from_temp() repeated" ) ) {
    PRINT_TEST;
    // Runs set_display_from_temp() several times to ensure it
    // functions properly in sequence
    printf("FIRST RUN\n");
    temp->tenths_degrees = 563;
    temp->temp_mode      = 2;         // fahrenheit
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();

    printf("\n");
    printf("SECOND RUN\n");
    temp->tenths_degrees = -73;
    temp->temp_mode      = 1;         // celsius
    THERMO_SENSOR_PORT  = 0;          // ports should be ignored
    THERMO_STATUS_PORT  = 0b00000000;
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_temp() + set_display() negative" ) ) {
    PRINT_TEST;
    // Calls set_temp() and set_display() in sequence
    // Check that negative temperatures print properly
    THERMO_SENSOR_PORT  = (-234+450)*32 + 13;
    THERMO_STATUS_PORT  = 0b00101000; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_temp() + set_display() error" ) ) {
    PRINT_TEST;
    // Calls set_temp() and set_display() in sequence but 
    // sensor value is negative indicating an error
    THERMO_SENSOR_PORT  = -128;
    THERMO_STATUS_PORT  = 0b11101000; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_temp() + set_display() error" ) ) {
    PRINT_TEST;
    // Calls set_temp() and set_display() in sequence
    THERMO_SENSOR_PORT  = 900*32+1;
    THERMO_STATUS_PORT  = 0b11000100; // celsius+error
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test( "set_temp() + set_display() normal" ) ) {
    PRINT_TEST;
    // Calls set_temp() and set_display() in sequence
    THERMO_SENSOR_PORT  = (234+450)*32 + 13;
    THERMO_STATUS_PORT  = 0b00000000; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_set_temp_from_ports(temp);
    printf("ret: %d\n",ret);
    print_temp( temp );
    ret = CALL_set_display_from_temp(*temp, dispint);
    printf("ret: %d\n",ret);
    printf("%-19s : %s\n", "dispint",bitstr(*dispint,&dispspec));
    printf("%-19s : %s\n", "index",bitstr_index(&dispspec));
    print_ports();
    printf("SIMULATED DISPLAY:\n");
    THERMO_DISPLAY_PORT = *dispint;
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() positive temps") ) {
    PRINT_TEST;
    // Runs thermo_update() on min sensor value.
    THERMO_SENSOR_PORT  = (234+450)*32 + 13;
    THERMO_STATUS_PORT  = 0b00000000; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() positive temps") ) {
    PRINT_TEST;
    // Runs thermo_update() on min sensor value.
    THERMO_SENSOR_PORT  = (234+450)*32 + 13;
    THERMO_STATUS_PORT  = 0b00100000; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() negative temps") ) {
    PRINT_TEST;
    // Runs thermo_update() on min sensor value.
    THERMO_SENSOR_PORT  = (-78+450)*32 + 7;
    THERMO_STATUS_PORT  = 0b00000000; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() negative temps") ) {
    PRINT_TEST;
    // Runs thermo_update() on min sensor value.
    THERMO_SENSOR_PORT  = (-78+450)*32 + 7;
    THERMO_STATUS_PORT  = 0b00100000; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() negative temps") ) {
    PRINT_TEST;
    // Runs thermo_update() on min sensor value.
    THERMO_SENSOR_PORT  = (-356+450)*32 + 13;
    THERMO_STATUS_PORT  = 0b00000000; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() negative temps") ) {
    PRINT_TEST;
    // Runs thermo_update() on min sensor value.
    THERMO_SENSOR_PORT  = (-356+450)*32 + 13;
    THERMO_STATUS_PORT  = 0b00100000; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() above 100 F") ) {
    PRINT_TEST;
    // Runs thermo_update() on min sensor value.
    THERMO_SENSOR_PORT  = (419+450)*32 + 18;
    THERMO_STATUS_PORT  = 0b00100000; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() min/max") ) {
    PRINT_TEST;
    // Runs thermo_update() on min and max sensor values.
    THERMO_SENSOR_PORT  = 0;
    THERMO_STATUS_PORT  = 0b00000000; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() min/max") ) {
    PRINT_TEST;
    // Runs thermo_update() on min and max sensor values.
    THERMO_SENSOR_PORT  = 900*32;
    THERMO_STATUS_PORT  = 0b00000000; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() min/max") ) {
    PRINT_TEST;
    // Runs thermo_update() on min and max sensor values.
    THERMO_SENSOR_PORT  = 0;
    THERMO_STATUS_PORT  = 0b00100000; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST


  if( run_test("thermo_update() min/max") ) {
    PRINT_TEST;
    // Runs thermo_update() on min and max sensor values.
    THERMO_SENSOR_PORT  = 900*32;
    THERMO_STATUS_PORT  = 0b00100000; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() status nonzeros") ) {
    PRINT_TEST;
    // Checks that nonzeros in the status port are ignored
    // (except bit 5 for fahrenheit and 2 for error state)
    THERMO_SENSOR_PORT  = (367+450)*32+19;
    THERMO_STATUS_PORT  = 0b11100001; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() status nonzeros") ) {
    PRINT_TEST;
    // Checks that nonzeros in the status port are ignored
    // (except bit 5 for fahrenheit and 2 for error state)
    THERMO_SENSOR_PORT  = (-367+450)*32+19;
    THERMO_STATUS_PORT  = 0b11000010; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() error range") ) {
    PRINT_TEST;
    // Checks that sensor values out of range are handled
    // correctly and display ERR
    THERMO_SENSOR_PORT  = 901*32;
    THERMO_STATUS_PORT  = 0b00100000; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST


  if( run_test("thermo_update() error range") ) {
    PRINT_TEST;
    // Checks that sensor values out of range are handled
    // correctly and display ERR
    THERMO_SENSOR_PORT  = -17;
    THERMO_STATUS_PORT  = 0b00000000; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() error status") ) {
    PRINT_TEST;
    // Checks that bit 2 of the status port is checked to
    // see if other internal errors have occurred.
    THERMO_SENSOR_PORT  = (230+450)*32 + 1;
    THERMO_STATUS_PORT  = 0b00000100; // celsius
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  if( run_test("thermo_update() error status") ) {
    PRINT_TEST;
    // Checks that bit 2 of the status port is checked to
    // see if other internal errors have occurred.
    THERMO_SENSOR_PORT  = (-123+450)*32 + 1;
    THERMO_STATUS_PORT  = 0b11100100; // fahrenheit
    THERMO_DISPLAY_PORT = -1;
    ret = CALL_thermo_update();
    printf("ret: %d\n",ret);
    print_ports();
    print_thermo_display();
    check_callee_registers();
  } // ENDTEST

  free(dispint);
  free(temp);

  return 0;
}

