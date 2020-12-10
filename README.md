# AVR-RAM_and_FLASH_tester

1. [Introduction](#Introduction)
2. [Tester algorithms](#Tester-algorithms)
3. [Using this library](#Using-this-library)
4. [Built-in example](#Built-in-example)
5. [Known issues](#Known-issues)
6. [Future improvements](#Future-improvements)


## Introduction

This library (/lib/avr_test_ram_and_flash) is meant to be used for testing if the RAM and Flash memory of an ATMEGA MCU is corrupted, and it has the hability to insert faults (as explained in [Using this library](#Using-this-library)).
It can be used with an already existing program with minimal modifications.

This tester has been developed and verified for the ATMEGA328P present in the Arduino UNO (or equivelent clones) board, but with some little modifications (mainly the #defines in the .c file of the library) it can be ported to other ATMEGA MCUs or other AVR ASM compatible MCUs.

Since this library was developed with PlatformIO, for you to use it standalone all you need to do is copu the files in (/lib/avr_test_ram_and_flash).

## Tester algorithms
### RAM

The algorithm used for testing the ATMEGA's RAM is the MATS++, and irredundant march test algorithm, that operates in the following way: 
<p align="center">
{⇕ (w(0)); ⇑ r(0),w(1)); ⇓ (r(1) , w(0), r(0)) } <br>
 </p>
 <p align="center">
 Each operation is done for all addresses before doing the next <br>
 </p>
 <p align="center">
  <b>Explanation of the symbols:</b> <br>
 ⇕ - move up or down and address (up in our implementation) <br>
 ⇑ - move up an address <br>
 ⇓ - move down an address <br>
 w(X) - write X to the current address <br>
 r(X) - read from current address and compare with X (error if they don't match)

</p>


There are other algorithms that have different properties (fault coverage):

<p align="center">
  <img width="460" height="300" src="/img/ram_tests_table.jpeg">
</p>
<p align="center">
  <sub>
Note: we apologise for the low quality image but it was the only that we could find
  </sub>
</p>


As you can see by the table this algorithm doesn't cover all typed of faults, so if you need a more complete coverage of faults we would recommend the MARCH C-, although this has almost double the number of operations than the algorithm used in this tester. As with most things, choose the one that better suits your needs and requirements.

### Flash



## Using this library 

In order to use this library all you need to do is import the .h file and call the testFlash() and testRam() whenever you need them. Note that the RAM function has arguments : testRam(uint8_t base, uint8_t top), and unless you know what you are doing never set the top value to an high value because it might corrupt the stack, so if you pass it 0 it will ignore the stack. 

In the .h file there is the possibility to test the RAM and the Flash before main is called (by the order they're presented here) by uncomenting (defining) the following #define:
* TEST_FLASH (value doesn't matter) - it saves the checksum in the ATMEGA's EEPROM at the address CHECKSUM_EEPROM_ADDRESS. In the case that you want to reset the value in the EEPROM uncoment (define) RESET_CHECKSUM and program and run the program once, then coment (undefine) it to use it again.
* TEST_RAM (value doesn't matter unless you intend to inject faults (will be explained next))

In the .h file, if the previously mentioned enviroment variables are defined you can inject faults by uncomenting (defining) the following #define:
* FLASH_INJECT_FAULT (value sets the address to corrupt, DO NOT SET TO VALUE OCCUPIED BY THE BOOTLOADER) 
* RAM_INJECT_FAULT (value sets the address to corrupt) - if TEST_RAM is set to 0 it will corrupt the RAM between the first and second operation of the MATS++ algorithm and if you set it to any other value it will corrupt the RAM between the second and last operation



## Built-in example
## Known issues

1. For some reason the return types and arguments of the functions cannot be defined by #define (they give a "does not name a type" error), maybe this is a limitation of the version of the AVR's GCC compiler used by the Arduino IDE. So because of this in order to be used with some other ATMEGA MCU with different RAM and/or Flash configurations you may need to modify these parts accordingly. The same thing applies for the checksum algorithm, if you want a different size checksum you'll need to change the argument of the generateChecksum() function and maybe some internal variable sizes.

## Future improvements

1. Fix the issue with the #defines mentioned in the first point of the [Known issues](#Known-issues)
2. Integrate the testing of the stack (right now you'll need to first run testRam(RAM_BASE_ADDRESS, 0) and then backup the stack to another part of ram and then run testRam(0, RAM_TOP_ADDRESS). As you can see this isn't the greatest implementation
3. The saving of the checksum value to the ATMEGA's EEPROM assumes that the checksum is 16 bit, so that part needs to be extended for other values (such as 8, 32 and 64 bit, they are natively supported by the AVR-GCC)
