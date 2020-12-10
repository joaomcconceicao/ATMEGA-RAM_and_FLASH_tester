#include <avr_test_ram_and_flash.h>

#define TEST_RAM 1 // uncomment to test the RAM (if RAM_INJECT_FAULT is defined: 0 for r0 part other value for r1 part of MATS++)
//#define RAM_INJECT_FAULT 0x0000  // uncomment to inject a fault (value sets the address to be corrupted)

#define TEST_FLASH 1 // uncomment to test the FLASH (value doesn't matter)
//#define FLASH_INJECT_FAULT 0x0000 // uncomment to inject a fault (value sets the address (try to use low values (on the ATMEGA328p >0x00FF) because it might corrupt the bootloader !!!))

#define RESET_CHECKSUM 1

int main(){

 
  
}