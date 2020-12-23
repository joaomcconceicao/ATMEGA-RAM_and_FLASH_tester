/*
	ATMEGA RAM and FLASH tester example 	Rev 1.0
	Developed by: Francisco Oliveira & Joao Conceicao
   	17/12/2020

    CHECK THE GITHUB'S README FOR AN EXPLANATION OF HOW TO USE THIS EXAMPLE
*/

#include <avr_test_ram_and_flash.h>

FLASH_CHECKSUM_SIZE PROGRAM_CHECKSUM = 0x0000;

void TestError(void){
	
	DDRB |= (1<<PB5);
	PORTB |= (1<<PB5);
	
	while(1);
}



int main(void)
{

}

