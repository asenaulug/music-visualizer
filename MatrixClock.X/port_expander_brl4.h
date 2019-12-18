/* 
 * File:   port_expander.h
 * Author: Sean
 *
 * Created on October 3, 2016, 10:28 AM
 */

#ifndef PORT_EXPANDER_H
#define	PORT_EXPANDER_H
/* Library for interacting with MCP23S17 port expander */
#include "plib.h"

#define PE_OPCODE_HEADER 0b01000000
#define READ 0b00000001
#define WRITE 0b00000000

// IOCON Settings
#define SET_BANK     0x80
#define CLEAR_BANK   0x00
#define SET_MIRROR   0x40
#define CLEAR_MIRROR 0x00
#define SET_SEQOP    0x20
#define CLEAR_SEQOP  0x00
#define SET_DISSLW   0x10
#define CLEAR_DISSLW 0x00
#define SET_HAEN     0x08
#define CLEAR_HAEN   0x00
#define SET_ODR      0x04
#define CLEAR_ODR    0x00
#define SET_INTPOL   0x02
#define CLEAR_INTPOL 0x00


// **** Register Addresses (BANK=0) ****
// Note: Rename all PortA to PortY, PortB to PortZ to avoid confusion with PIC
#define IODIRY   0x00
#define IODIRZ   0x01
#define IPOLY    0x02
#define IPOLZ    0x03
#define GPINTENY 0x04
#define GPINTENZ 0x05
#define DEFVALY  0x06
#define DEFVALZ  0x07
#define INTCONY  0x08
#define INTCONZ  0x09
#define IOCON    0x0A
//#define IOCON    0x0B
#define GPPUY    0x0C
#define GPPUZ    0x0D
#define INTFY    0x0E
#define INTFZ    0x0F
#define INTCAPY  0x10
#define INTCAPZ  0x11
#define GPIOY    0x12
#define GPIOZ    0x13
#define OLATY    0x14
#define OLATZ    0x15

/* Initializes SPI channel 2 with 8-bit modeRB9 for CS: 
 * pins used:
 *  --  CS - RPB9 (Pin 18)
 *  -- SCK - SCK2 (Pin 26)
 *  -- SDI - RPB2 (Pin  6)
 *  -- SDO - RPB5 (Pin 14)
 * Sets the clock divisor for pb_clock to 4, giving 10MHz (fastest possible for
 * our MCP23S17 package)
 */
void initPE();

void mPortYSetPinsOut(unsigned char);

void mPortZSetPinsOut(unsigned char);

void mPortYSetPinsIn(unsigned char);

void mPortZSetPinsIn(unsigned char);

void mPortYIntEnable(unsigned char);

void mPortYIntDisable(unsigned char);

void mPortZIntEnable(unsigned char);

void mPortZIntDisable(unsigned char);

void mPortYEnablePullUp(unsigned char);

void mPortZEnablePullUp(unsigned char);

void mPortYDisablePullUp(unsigned char);

void mPortZDisablePullUp(unsigned char);


/* Takes a register address on port expander and a data byte, and writes the 
 * data to the target register. */
inline void writePE(unsigned char, unsigned char);

/* Takes a register address on port expander and returns the data byte from that
 * target register. */
inline unsigned char readPE(unsigned char);

// spi2 lock so that port expander OR DAC can use the channel
extern volatile int spi2_lock ;
#endif	/* PORT_EXPANDER_H */

