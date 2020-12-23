# AVR-RAM_and_FLASH_tester

1. [Introduction](#Introduction)
2. [Tester algorithms](#Tester-algorithms)
3. [Using this library](#Using-this-library)
4. [Built-in example](#Built-in-example)
5. [Known issues](#Known-issues)
6. [Future improvements](#Future-improvements)
7. [Previous versions](#Previous-versions)

## Introduction

This library (/lib/avr_test_ram_and_flash) is meant to be used for testing if the RAM and Flash memory of an ATMEGA MCU is corrupted, and it has the hability to insert faults (as explained in [Using this library](#Using-this-library)).
It can be used with an already existing program with minimal modifications.

This tester has been developed and verified for the ATMEGA328P present in the Arduino UNO (or equivelent clones) board, but with some little modifications (mainly the #defines in the .c file of the library) it can be ported to other ATMEGA MCUs or other AVR ASM compatible MCUs.

Since this library was developed with PlatformIO, for you to use it standalone all you need to do is copu the files in (/lib/avr_test_ram_and_flash).

## Tester algorithms

### Flash

#### Explanation

The algorithm used for this is the CRC (Cyclic Redundancy Check) that is extensively explained in an Aplication Note by ATMEL (now Microchip), the makers of the ATMEGA. This explains way better and in more detail than we would be able to do here, so go to the provided link and read it if you're interested.

[AVR236: CRC Check of Program Memory; ATMEL/MICROCHIP Aplication Note](http://ww1.microchip.com/downloads/en/AppNotes/doc1143.pdf)

#### Notes

The way this algorithm was implemented it only uses registers, so the issues with the RAM won't affect this tester.

### RAM

#### Explanation

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

#### Notes

The way this algorithm was implemented it only uses registers, so the issues with the RAM won't affect this tester.


## Using this library 

By default the library will test the RAM and the FLASH before calling main, so it's easier to integrate with existing programs. For this GCC's function attributes are used, and this is considered not portable, so if this doesn't work for you erase the  __attribute__((constructor)) and just call the function in the first lines of your program's main function.

You'll need to define an error function, that needs to be <b>void TestError()</b> in your program, that will be called when an error is deteted in your program. If you don't do this the program will compile but it will have an undefined behaviour.

You also need to define the program's checksum and for that use the python script present in the GitHu

If you want to disable any of the testers, just define the according XXX_DISABLE_TESTER in the .h file or in your program.

If you want to inject faults, see the .h file for an explanation.

The Flash test is ran first because if the Flash memory is corrupted it is of no use to continue running the program.

## Built-in example

The example given is the main.cpp file, that as you can se only includes the library and the important checksum and the error function.
It is meant to test the RAM and the Flash before executing the main function so if you want add whatever code you'd like. (Calculate the Checksum before programing the device !!!)
This example is meant to be used with an ATMEGA328P Arduino UNO (or clone equivelent) board !

Watch the User LED (connected to pin 13), if the test fails it stays on otherwise it stays off.

To use it do the following:

1. Program and run the program as given
2. The ATMEGA's RAM and Flash will be tested, but if you MCU isn't faulty the LED won't turn on
3. Properly define any of the fault injections mentioned in the .h file
4. Program and run the program again
5. The LED will stay on because the tester found an error

## Known issues

1. Doesn't test the SRAM memory that is part of the stack

## Future improvements

1. Allow for the testing of the SRAM memory that is part of the stack

## Previous versions

In the previous version there was a couple of problems that made the tester lock up the MCU when they were ran:

1. In the RAM tester the calculation of the stack pointer address was wrong, it made it so that the lower and upper byte were flipped, so the tester tested the whole memory of the ATMEGA and that would corrupt both the registers and the stack.
2. In the FLASH memory the was an error in the ammount of steps to be made by each iteration of the algorithm, and it would always give 0

Also in the previous version the initial checksum was calculated in the device itself, and since the device's FLASH memory could already be corrupted, the calculated checksum would be invalid, unless if a new corruption occured after that initial calculation. This is clearly wrong. The EEPROM was also used to store the checksum, and that wasn't correct, since the EEPROM itself could be corrupted, and that memory is outside the scope of this project.
