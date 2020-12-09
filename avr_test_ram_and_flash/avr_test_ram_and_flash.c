/*
	ATMEGA RAM and FLASH tester 	Rev 0.1
	Developed by: Francisco Oliveira & Joao Conceicao
	02/12/2020
	
	Explanation:
	
		- Software to test the integrity of RAM and FLASH memory on ATMEGA devices, using the MATS++ algorithm for testing RAM and using a checksum to test the FLASH in order to preserve the lifespan of the FLASH memory
	
		- Supports injection of faults:
			-> for RAM: uncomment TEST_RAM 
				-define (uncomment) RAM_INJECTION_FAULT and the value will be the address to be corrupted (the address is relative to RAM_BASE_ADDRESS)
				-set TEST_RAM to 0 for r0 part of the MATS++ algorithm and set to other value for the r1 part of MATS++
			-> for FLASH: not yet implemented
	
	Tested on:
		- ATMEGA328P
	
	NOTE: 	- It may work on different ATMEGA ICs but the RAM and FLASH addresses need to be changed accordingly !!!
		- The testError() function is only to be used as is if the software is to be used in an Arduino UNO platform (ATMEGA328P), if you want to use it differently change the code inside to suit your needs
	
	TODO: 	
		- create fault injection for FLASH memory (maybe an ifdef that injects an extra line of code in the program)
		- add relevant comments
		- format the code properly (make it pretty and readable)
*/

#include <stdint.h>	// needed for using data types (C's default ones are implementation dependent so this method is better)
#include <avr/pgmspace.h>	// needed for abstracting the access of the FLASH memory

//#include "avr_test_ram_and_flash.h"

#include <util/delay.h>		// needed for the error function, if you change the function and you don't need delays delete this line !!!

#define RAM_SIZE 2048 // Number of addresses in the ATMEGA's SRAM 
#define RAM_BASE_ADDRESS 0x0100	// SRAM base address (the lower values are reserved for the ATMEGA's registers)
#define RAM_TOP_ADDRESS (RAM_BASE_ADDRESS + RAM_SIZE) // SRAM top address, calculated from the base address and size
#define RAM_POINTER uint8_t	// The size of the SRAM elements (on ATMEGA328P is 8 bit, change if your platform is different)  (ATMEGA328P SRAM size = 2048 x 8bit)

#define FLASH_SIZE 0x3FFF // Number of FLASH memory addresses in the ATMEGA's FLASH
#define FLASH_BASE_ADDRESS 0x0000 // FLASH base address
#define FLASH_TOP_ADDRESS (FLASH_BASE_ADDRESS + FLASH_SIZE) // FLASH top address, calculated from the base address and size
#define FLASH_POINTER uint16_t // The size of the FLASH elements (on ATMEGA328P is 16 bit, change if your platform is different) (ATMEGA328P FLASH size = 32k x 16bit)
#define FLASH_CHECKSUM uint16_t // Size of the checksum

#define CHECKSUM_DETECT_CARRY 0x8000


#define PROGRAM_CHECKSUM 0xFFFF // Program's checksum (should be defined in the main program ??)

uint16_t generateChecksum(){

	register FLASH_POINTER flash_pointer = (FLASH_POINTER)(FLASH_BASE_ADDRESS);	// loads the base address of the flash memory into a register
	register FLASH_CHECKSUM checksum_divisor = 0x8005;
	register uint8_t bit_counter = 0x11;
	register FLASH_CHECKSUM checksum = 0;						// initializes the checksum register
	register FLASH_CHECKSUM checksum_data_to_add;
	
	for (flash_pointer = FLASH_BASE_ADDRESS; flash_pointer < (FLASH_POINTER)(FLASH_TOP_ADDRESS); flash_pointer++){	// sweeps trough the whole flash memory
		
		checksum_data_to_add = (FLASH_CHECKSUM) pgm_read_word_near(flash_pointer);		// calculates the checksum
		
		for(checksum_divisor; checksum_divisor != 0; checksum_divisor--){

			if(checksum_data_to_add & CHECKSUM_DETECT_CARRY)
				break;
			
			checksum_data_to_add = checksum_data_to_add << 1;
		}

		checksum ^= (FLASH_CHECKSUM) (checksum_data_to_add << 1);		// calculates the checksum
	}

	return checksum;
}

int8_t testFlash(uint16_t checksum){

	if(generateChecksum() != checksum)	// if the checksum doesn't match jump the the error function (maybe discriminate RAM errors from FLASH errors ? )
		return -1;

  	return 0;
}


int8_t testRam(uint8_t base, uint8_t top){

		// This tester is based on the MATS++ testing algorithm, so it doesn't detect all types of faults !!!
	
	// IMPORTANT NOTES ABOUT THIS TESTER:
	// 	- it uses only registers for its operation, so the lack of integrity of the ram doesn't matter
	
    register RAM_POINTER* ram_pointer = (RAM_POINTER*) 0x3D;
	register RAM_POINTER* stack_pointer = !top ? (RAM_POINTER*) ( *(ram_pointer)<<8 | *(ram_pointer + 1)) : (RAM_POINTER*) top;
	
    for (ram_pointer =  (RAM_POINTER *)base; ram_pointer < (RAM_POINTER *)((RAM_POINTER *)stack_pointer); ram_pointer++)	// sweeps trough the whole ram memory
        *ram_pointer = (RAM_POINTER) 0;											// writes all 0's to the current address

#ifdef RAM_INJECT_FAULT		// this code is used to inject faults (see top of page to see explanation) 			
  	
	if(!TEST_RAM){
		ram_pointer = (RAM_POINTER *)(RAM_INJECT_FAULT + RAM_BASE_ADDRESS);
  		*ram_pointer = (RAM_POINTER) 0;
	}

#endif

	for (ram_pointer =  (RAM_POINTER *)base; ram_pointer < (RAM_POINTER *)((RAM_POINTER *)stack_pointer); ram_pointer++){ // sweeps trough the whole ram memory

      if(*ram_pointer != (RAM_POINTER) 0)										// checks if the current address has the expected value (all 0's) if not it goes to the error function
      		return -1;
    	
      *ram_pointer = (RAM_POINTER) 0xFF;										// writes all 1's to the current address
      }

  
#ifdef RAM_INJECT_FAULT		// this code is used to inject faults (see top of page to see explanation) 			
  	
	if(!TEST_RAM){
		ram_pointer = (RAM_POINTER *)(RAM_INJECT_FAULT + RAM_BASE_ADDRESS);
  		*ram_pointer = (RAM_POINTER) 0;
	}

#endif
 
  	for (ram_pointer =  (RAM_POINTER *)base; ram_pointer < (RAM_POINTER *)((RAM_POINTER *)stack_pointer); ram_pointer++){ // sweeps trough the whole ram memory

        if(*ram_pointer != (RAM_POINTER) 0xFF) // checks if the current address has the expected value (all 1's) if not it goes to the error function
 			return -1;
        
    }

	
	return 0;	

	
}
