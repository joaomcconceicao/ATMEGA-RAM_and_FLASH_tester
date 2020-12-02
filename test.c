#include <stdint.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define RAM_SIZE 2048
#define RAM_BASE_ADDRESS 0x0100
#define RAM_TOP_ADDRESS (RAM_BASE_ADDRESS + RAM_SIZE)
#define RAM_POINTER uint8_t

#define FLASH_SIZE 0x3FFF
#define FLASH_BASE_ADDRESS 0x0000
#define FLASH_TOP_ADDRESS (FLASH_BASE_ADDRESS + FLASH_SIZE) - 10
#define FLASH_POINTER uint8_t
#define FLASH_CHECKSUM uint16_t

#define PROGRAM_CHECKSUM 0xFFFF

//#define RAM_INJECT_FAULT 0x0000

#define TEST_RAM 1
#define TEST_FLASH 1

void testError(){
  
 DDRB |= (1<<PB5);
  
  while(1){
   PORTB |= (1<<PB5); 
   _delay_ms(1000); 
    PORTB &= ~(1<<PB5);
   _delay_ms(1000);  
  }
  
}

#ifdef TEST_FLASH
void __attribute__((constructor)) TestFLASH(){
 
  register FLASH_POINTER flash_pointer = (FLASH_POINTER)(FLASH_BASE_ADDRESS);
  register FLASH_CHECKSUM checksum = 0;
  
  for (flash_pointer = FLASH_BASE_ADDRESS; flash_pointer < (FLASH_POINTER)(FLASH_TOP_ADDRESS); flash_pointer++)
    checksum += (FLASH_CHECKSUM) pgm_read_byte_near(flash_pointer);
  
  if(checksum == PROGRAM_CHECKSUM)
    testError();

  return;
}
#endif

#ifdef TEST_RAM
void __attribute__((constructor)) TestRAM() {

    register RAM_POINTER *ram_pointer = (RAM_POINTER*)(RAM_BASE_ADDRESS);

    for (ram_pointer = (RAM_POINTER *) RAM_BASE_ADDRESS; ram_pointer < (RAM_POINTER*)(RAM_TOP_ADDRESS); ram_pointer++)
        *ram_pointer = (RAM_POINTER) 0;
    
	for (ram_pointer = (RAM_POINTER *) RAM_BASE_ADDRESS; ram_pointer < (RAM_POINTER*)(RAM_TOP_ADDRESS); ram_pointer++){

      if(*ram_pointer != (RAM_POINTER) 0x00)
      		testError();
    	
      *ram_pointer = (RAM_POINTER) 0xFF;
      }

  
#ifdef RAM_INJECT_FAULT  
  	ram_pointer = (RAM_POINTER *)(RAM_INJECT_FAULT + RAM_BASE_ADDRESS);
  	*ram_pointer = (RAM_POINTER) 0x00;
#endif
 
  	for (ram_pointer = (RAM_POINTER *) RAM_BASE_ADDRESS; ram_pointer < (RAM_POINTER*)(RAM_TOP_ADDRESS); ram_pointer++){

        if(*ram_pointer != (RAM_POINTER) 0xFF)
 			testError();
        
    }

   asm("jmp main \n\t");
 } 
#endif

int main (void){
	  

}
