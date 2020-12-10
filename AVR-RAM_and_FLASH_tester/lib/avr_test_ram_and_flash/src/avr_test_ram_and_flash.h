/*
	ATMEGA RAM and FLASH tester 	Rev 0.4
	Developed by: Francisco Oliveira & Joao Conceicao
	07/12/2020
	

	Explanation:
	
		- Software to test the integrity of RAM and FLASH memory on ATMEGA devices, using the MATS++ algorithm for testing RAM and using a checksum (CRC) to test the FLASH in order to preserve the lifespan of the FLASH memory
	
		- Supports injection of faults:

			-> for RAM: uncomment TEST_RAM 
				- define (uncomment) RAM_INJECTION_FAULT and the value will be the address to be corrupted (the address is relative to RAM_BASE_ADDRESS)
				- set TEST_RAM to 0 for r0 part of the MATS++ algorithm and set to other value for the r1 part of MATS++
				- program and run the program
				- if the test fails the PB5 pin (on an Arduino Uno board it's the user LED) will blink forever (the program will be stuck in a loop forever !)

			-> for FLASH: uncomment TEST_FLASH

				You'll need to program the ATMEGA twice:
					1.
						- define (uncomment) RESET_CHECKSUM (value doesn't matter) and the checksum for your program will be written to the EEPROM address CHECKSUM_EEPROM_ADDRESS
						- program and run the program once
					2.
						- undefine (comment) RESET_CHECKSUM
						- define (uncomment) FLASH_INJECTION_FAULT and the value will be the address to be corrupted (the address is relative to FLASH_BASE_ADDRESS (use only addresses not used by the bootloader !!!!)) 
						- program and run the program
						- if the test fails the PB5 pin (on an Arduino Uno board it's the user LED) will blink forever (the program will be stuck in a loop forever !)
	
	Tested on:
		- ATMEGA328P
	
	NOTE: 	- It may work on different ATMEGA ICs but the RAM and FLASH addresses need to be changed accordingly !!!
	
	TODO: 	
		- for some reason using the #define has a return types for functions or as arguments gives a does not name a type error
*/


#include <stdint.h>
#include <avr/eeprom.h>     // needed for writing and reading from the EEPROM
#include <avr/boot.h>       // needed for abstracting the writing of faults to the FLASH memory
#include <avr/pgmspace.h>	// needed for abstracting the read of the FLASH memory
#include <stdint.h>	// needed for using data types (C's default ones are implementation dependent so this method is better)

#include <util/delay.h> // used for the default testError() function (to blink PB5 every second), if you change the function you can delete this
#define F_CPU   1600000UL	// Change to your MCU's clock frequency (only needed if you use the default testError() function, otherwise you can delete this)

//#define TEST_RAM 1 // uncomment to test the RAM (if RAM_INJECT_FAULT is defined: 0 for r0 part other value for r1 part of MATS++)
//#define RAM_INJECT_FAULT 0x0000  // uncomment to inject a fault (value sets the address to be corrupted)

//#define TEST_FLASH 1 // uncomment to test the FLASH (value doesn't matter)
//#define FLASH_INJECT_FAULT 0x0000 // uncomment to inject a fault (value sets the address (try to use low values (on the ATMEGA328p >0x00FF) because it might corrupt the bootloader !!!))

//#define RESET_CHECKSUM 1
 
#define CHECKSUM_EEPROM_ADDRESS 0x0000	// The EEPROM address to save the Flash checksum

int8_t testFlash();
int8_t testRam(uint8_t base, uint8_t top);
uint16_t generateChecksum();


// Function that is called if you use the tests before executing main, in this case it only blinks the LED
void testError(){

    DDRB |= (1<<PB5);
	while(1){
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

#ifdef  RESET_CHECKSUM

        eeprom_write_word(CHECKSUM_EEPROM_ADDRESS, generateChecksum());	// Writes the Flash contents checksum to the EEPROM at address CHECKSUM_EEPROM_ADDRESS

#endif
#ifndef RESET_CHECKSUM		

#ifdef FLASH_INJECT_FAULT

    boot_page_fill (FLASH_INJECT_FAULT, 0xFFFF);	// writes all 1s to the address FLASH_INJECT_FAULT (all 1's is highly unlikely to occur so it's a good value)
    boot_spm_busy_wait ();  			// waits for the flash write to be done

#endif
        
		if(testFlash((uint16_t)eeprom_read_word(CHECKSUM_EEPROM_ADDRESS)))	// Checks if the checksum in the EEPROM is correct
			testError();
#endif
	}
#endif

#ifdef TEST_RAM	// uncomment TEST_RAM to test the ram (0 for r0 part other value for r1 part of MATS++)
	void __attribute__((constructor)) TestRAM() {  // this method is not portable it may not work on all compilers !!! it serves to call this function before the execution of the main function

		if(testRam(RAM_BASE_ADDRESS, 0))	// checks if the RAM isn't corrupted
            testError();
		
	} 
#endif
