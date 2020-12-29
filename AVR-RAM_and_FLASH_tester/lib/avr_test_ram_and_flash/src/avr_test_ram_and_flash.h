/*
	ATMEGA RAM and FLASH tester 	Rev 1.0
	Developed by: Francisco Oliveira & Joao Conceicao
	17/12/2020
	
	To see how to use this library check the Github. 

	Explanation:
	
		- Software to test the integrity of RAM and FLASH memory on ATMEGA devices, using the MATS++ algorithm for testing RAM and using a checksum (CRC) to test the FLASH in order to preserve the lifespan of the FLASH memory
	
		- Supports injection of faults
		
		
	Tested on:
		- ATMEGA328P
	
	NOTE: 	- It may work on different ATMEGA ICs but the RAM and FLASH addresses need to be changed accordingly !!!
	
*/

//#define RAM_DISABLE_TESTER 0		// Define to disable the RAM tester

// Fault injection: Both of these need to be defined in order to inject faults!!
//#define RAM_INJECT_FAULT_STEP 0	// Define to select in which step the fault is to be injected (before each of the read operations of the MATS++ test 
//#define RAM_INJECT_FAULT_ADDRESS 0 	// Define to select in which address the fault is to be injected, this address will be offset ed by RAM_BASE_ADDRESS addresses

//#define FLASH_DISABLE_TESTER 0	// Define to disable the FLASH tester
//#define FLASH_INJECT_FAULT_ADDRESS 0	// Defines in which address to inject the fault (for simplicity the fault injector inverts the value read)

#include <avr/io.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

#define RAM_SIZE 2048					// Size of the RAM in RAM_POINTER addresses 
#define RAM_BASE_ADDRESS 0x0100				// Base address of the SRAM (do not put an address that is part of the registers, as this will stop the program)
#define RAM_TOP_ADDRESS (RAM_BASE_ADDRESS + RAM_SIZE)	// Top address of the SRAM
#define RAM_POINTER uint8_t				// Size of the RAM values

#define FLASH_SIZE 0x3FFF 					// Size of the FLASH
#define FLASH_BASE_ADDRESS 0x0000				// Base address of the FLASH
#define FLASH_TOP_ADDRESS (FLASH_BASE_ADDRESS + FLASH_SIZE) 	// Top address of the FLASh
#define FLASH_POINTER uint16_t					// Size of the FLASH values
#define FLASH_CHECKSUM_SIZE uint16_t				// Size of the desired checksum
#define FLASH_CHECKSUM_BIT_COUNTER 0x11				// Number of maximum shifts to do in the CRC's algorithm
#define FLASH_CHECKSUM_DIVISOR 0x8005				// Divisor value in the CRC's algorithm
#define FLASH_CHECKSUM_DETECT_CARRY 0x8000			// Used to detect if the carry condition of the CRC's

extern void TestError(void);					// Function to be called when one of the tests fails, define in your program
	
extern const FLASH_POINTER 	PROGRAM_CHECKSUM_[1] PROGMEM;		// FLASH checksum value, declare in your program


#ifndef RAM_DISABLE_TESTER
void __attribute__((constructor)) __TEST_RAM() {		// This function will be the last one called before the execution of main (this method is not portable, so if you want to use as a normal function remove the  __attribute__((constructor)) and declare this function first in your program)

	register RAM_POINTER* current_address;			// Pointer to sweep trough the RAM addresses
	register RAM_POINTER* base_address = (RAM_POINTER*) RAM_BASE_ADDRESS; // Pointer to the base address of the RAM
	register RAM_POINTER* top_address = (RAM_POINTER*) ((SPH<<8)|(SPL));  // Pointer to the begining of the stack
	
	for(current_address = base_address; current_address < top_address; current_address++ ){	// First step of MATS++: write 0s to all addresses
		(*current_address) = (RAM_POINTER) 0;
	}
	
#ifdef RAM_INJECT_FAULT_STEP	// If you want to inject a fault in this part
	
	#ifdef RAM_INJECT_FAULT_ADDRESS
	
		if(RAM_INJECT_FAULT_STEP == 0){
			current_address = (RAM_POINTER*) RAM_INJECT_FAULT_ADDRESS;
			(*current_address) = (RAM_POINTER) -1;		// Write all 1's to the address
		}
		
	#endif
	
#endif
	
	for(current_address = base_address; current_address < top_address; current_address++ ){	// Second step of MATS++: read 0s and write 1s to all addresses, if the read 0s has an error goto TestError()
		
		if((RAM_POINTER) 0 != (*current_address)){
			TestError();
			return;
		}
		
		(*current_address) = (RAM_POINTER) -1;
	}

#ifdef RAM_INJECT_FAULT_STEP	// If you want to inject a fault in this part

	#ifdef RAM_INJECT_FAULT_ADDRESS

		if(RAM_INJECT_FAULT_STEP == 1){
			current_address = (RAM_POINTER*) RAM_INJECT_FAULT_ADDRESS;
			(*current_address) = (RAM_POINTER) 0;	// Write all 0's to the address
		}

	#endif

#endif
	
	for(current_address = top_address - 1; current_address > base_address; current_address-- ){ // Third step of MATS++: read 1s , write 0s and read 0s to all addresses, if the read 1s or the read 0s has an error goto TestError()
		
		
		if(((RAM_POINTER) -1) != (*current_address)){
			TestError();
			return;
		}
		
		(*current_address) = (RAM_POINTER) 0;

#ifdef RAM_INJECT_FAULT_STEP // If you want to inject a fault in this part

	#ifdef RAM_INJECT_FAULT_ADDRESS

		if(RAM_INJECT_FAULT_STEP == 2){
			current_address = (RAM_POINTER*) RAM_INJECT_FAULT_ADDRESS;
			(*current_address) = (RAM_POINTER) -1;	// Write all 1's to the address
		}

	#endif

#endif
		
		if((RAM_POINTER) 0 != (*current_address)){
			TestError();
			return;	
		}
	}
	
	return;
}

#endif


#ifndef FLASH_DISABLE_TESTER

void __attribute__((constructor)) __TEST_FLASH(){ // This function will be the first one called before the execution of main (this method is not portable, so if you want to use as a normal function remove the  __attribute__((constructor)) and declare this function first in your program)
	
	register FLASH_POINTER current_address = (FLASH_POINTER) FLASH_BASE_ADDRESS;		// Base address of FLASH
	register FLASH_POINTER top_address = (FLASH_POINTER) FLASH_TOP_ADDRESS;			// Top address of FLASH
	register FLASH_CHECKSUM_SIZE checksum = (FLASH_CHECKSUM_SIZE) 0;			// The programs checksum
	register uint8_t bit_counter;

	for(current_address; current_address < (FLASH_POINTER)(top_address); current_address++){			// The CRC's algorithm
		
		if(current_address == (FLASH_POINTER)PROGRAM_CHECKSUM_){
			checksum ^= 0x0000;
		}
#ifdef FLASH_INJECT_FAULT_ADDRESS		
		else if (current_address == FLASH_INJECT_FAULT_ADDRESS){
			checksum ^= ~(FLASH_CHECKSUM_SIZE) pgm_read_word_near(current_address);
		}
#endif		
		else{
			checksum ^= (FLASH_CHECKSUM_SIZE) pgm_read_word_near(current_address);
		}
		
		for(bit_counter = (uint8_t)FLASH_CHECKSUM_BIT_COUNTER; bit_counter > 0; bit_counter--){
			
			if( checksum & (1 << (sizeof(FLASH_CHECKSUM_SIZE)-1)) ){
				checksum ^= FLASH_CHECKSUM_DIVISOR;
			}else
				checksum = checksum << 1;
		}
		
	}

	if(checksum != pgm_read_word(PROGRAM_CHECKSUM_))
		TestError();
	
	return;
}

#endif

