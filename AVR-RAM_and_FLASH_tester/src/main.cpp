#include "avr_mem_test.h"

const FLASH_POINTER PROGRAM_CHECKSUM_[1] PROGMEM= {0xaaaa};	// Insert here the checksum calculated in the python checksum generator

// Define here what should be done if the testers find a fault
void TestError(void){
	
	DDRB |= (1<<PB5);
	PORTB |= (1<<PB5);
	
	while(1);
}


// Apart from the previous mandatory declarations, you can program whatever this tester will always automatically run before any program  
int main(void)
{
	
}

