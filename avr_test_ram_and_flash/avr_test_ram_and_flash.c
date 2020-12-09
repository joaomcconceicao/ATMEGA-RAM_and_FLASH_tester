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
		- create a program (probably python) that creates the correct checksum of the compiled program (needs to take into account the bootloader !!!!) and maybe adds it to the checksum line (simple regex or even simpler method)
		- create fault injection for FLASH memory (maybe an ifdef that injects an extra line of code in the program)
		- the testRAM() corrupts the stack in its test so maybe find a way to regenerate the stack before returning ? the way it's implemented leaves some dead code after the asm("jmp main \n\t"), the code responsible for the return of the testRAM() function doesn't do anything
		- add relevant comments
		- format the code properly (make it pretty and readable)
		- make this implementable as a library to make it portable
*/

#include <stdint.h>	// needed for using data types (C's default ones are implementation dependent so this method is better)
#include <avr/pgmspace.h>	// needed for abstracting the access of the FLASH memory


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

#define PROGRAM_CHECKSUM 0xFFFF // Program's checksum (should be defined in the main program ??)

//#define RAM_INJECT_FAULT 0x0000  // uncomment to inject a fault as explained in the top of the page

#define TEST_RAM 1 // uncomment to test the RAM (0 for r0 part other value for r1 part of MATS++)
#define TEST_FLASH 1 // uncomment to test the FLASH (value doesn't matter)

void testError(){	// Function called in the case of an error on testing RAM or FLASH
  
 DDRB |= (1<<PB5);	// Sets the Arduino pin 13 (on-board led) to output
  
  while(1){	// Turns the led on and off every 1000ms (1 second)
   PORTB |= (1<<PB5); 
   _delay_ms(1000); 
    PORTB &= ~(1<<PB5);
   _delay_ms(1000);  
  }
  
}

#ifdef TEST_FLASH	// uncomment TEST_FASH to test the flash (value doesn't matter)
void __attribute__((constructor)) TestFLASH(){ // this method is not portable it may not work on all compilers !!! it serves to call this function before the execution of the main function
 
	//this function is declared before the RAM tester because if the FLASH memory is corrupted the RAM tester might have the wrong code or corrupted code and it may give false positives or negatives
	// because of that this tester doesn't use RAM, all variables are stored on registers, and in doing so the integrity of the RAM doesn't matter (it kind of does because the return address is stored on the RAM's stack, so maybe this needs to be changed (store the return address on registers and do a jump maybe))
	
	
	register FLASH_POINTER flash_pointer = (FLASH_POINTER)(FLASH_BASE_ADDRESS);	// loads the base address of the flash memory into a register
	register FLASH_CHECKSUM checksum = 0;						// initializes the checksum register
  
	for (flash_pointer = FLASH_BASE_ADDRESS; flash_pointer < (FLASH_POINTER)(FLASH_TOP_ADDRESS); flash_pointer++)	// sweeps trough the whole flash memory
		checksum += (FLASH_CHECKSUM) pgm_read_word_near(flash_pointer);		// calculates the checksum
  
	if(checksum != PROGRAM_CHECKSUM)	// if the checksum doesn't match jump the the error function (maybe discriminate RAM errors from FLASH errors ? )
		testError();

  	return;
}
#endif

#ifdef TEST_RAM	// uncomment TEST_RAM to test the ram (0 for r0 part other value for r1 part of MATS++)
void __attribute__((constructor)) TestRAM() {  // this method is not portable it may not work on all compilers !!! it serves to call this function before the execution of the main function
	
	// This tester is based on the MATS++ testing algorithm, so it doesn't detect all types of faults !!!
	
	// IMPORTANT NOTES ABOUT THIS TESTER:
	// 	- it uses only registers for its operation, so the lack of integrity of the ram doesn't matter
	// 	- it corrupts the stack !!!
	// 	- it leaves some dead code (if you check the assembly code the return part of this function is never used so it's dead code)
	
    register RAM_POINTER *ram_pointer = (RAM_POINTER*)(RAM_BASE_ADDRESS);	// loads the base address of the ATMEGA's SRAM

    for (ram_pointer = (RAM_POINTER *) RAM_BASE_ADDRESS; ram_pointer < (RAM_POINTER*)(RAM_TOP_ADDRESS); ram_pointer++)	// sweeps trough the whole ram memory
        *ram_pointer = (RAM_POINTER) 0;											// writes all 0's to the current address

#ifdef RAM_INJECT_FAULT		// this code is used to inject faults (see top of page to see explanation) 			
  	
	if(!TEST_RAM){
		ram_pointer = (RAM_POINTER *)(RAM_INJECT_FAULT + RAM_BASE_ADDRESS);
  		*ram_pointer = (RAM_POINTER) 0;
	}
#endif

	for (ram_pointer = (RAM_POINTER *) RAM_BASE_ADDRESS; ram_pointer < (RAM_POINTER*)(RAM_TOP_ADDRESS); ram_pointer++){ // sweeps trough the whole ram memory

      if(*ram_pointer != (RAM_POINTER) 0)										// checks if the current address has the expected value (all 0's) if not it goes to the error function
      		testError();
    	
      *ram_pointer = (RAM_POINTER) 0xFF;										// writes all 1's to the current address
      }

  
#ifdef RAM_INJECT_FAULT		// this code is used to inject faults (see top of page to see explanation) 			
  	
	if(!TEST_RAM){
		ram_pointer = (RAM_POINTER *)(RAM_INJECT_FAULT + RAM_BASE_ADDRESS);
  		*ram_pointer = (RAM_POINTER) 0;
	}

#endif
 
  	for (ram_pointer = (RAM_POINTER *) RAM_BASE_ADDRESS; ram_pointer < (RAM_POINTER*)(RAM_TOP_ADDRESS); ram_pointer++){ // sweeps trough the whole ram memory

        if(*ram_pointer != (RAM_POINTER) 0xFF) // checks if the current address has the expected value (all 1's) if not it goes to the error function
 			testError();
        
    }

   asm("jmp main \n\t");	// jumps to the main() function; this is where the dead code problem resides. This is necessary because of the corruption of the stack, so the return address is lost.
	return;	// This is never called so all of the code (poping from stack and returning) related to this is never called ! It's dead code so it's dangerous and it's wasteful, so if a better implementation is found this will be imediatly replaced !!!!
 } 
#endif

int main (){
}
