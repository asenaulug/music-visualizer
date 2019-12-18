#include "port_expander_brl4.h"

#define SET_CS    {mPORTBSetBits(BIT_9);}
#define CLEAR_CS  {mPORTBClearBits(BIT_9);}

// === spi bit widths ====================================================
// hit the SPI control register directly, SPI2
// Change the SPI bit modes on the fly, mid-transaction if necessary
inline void SPI_Mode16(void){  // configure SPI2 for 16-bit mode
    SPI2CONSET = 0x400;
    SPI2CONCLR = 0x800;
}
// ========
inline void SPI_Mode8(void){  // configure SPI2 for 8-bit mode
    SPI2CONCLR = 0x400;
    SPI2CONCLR = 0x800;
}
// ========
inline void SPI_Mode32(void){  // configure SPI2 for 32-bit mode
    SPI2CONCLR = 0x400;
    SPI2CONSET = 0x800;
}

void initPE() {
  // control CS for DAC
  volatile SpiChannel pe_spi = SPI_CHANNEL2;
  volatile int spiClkDiv = 4; // 10 MHz max speed for this DAC
  mPORTBSetPinsDigitalOut(BIT_9); // use RPB9 (pin 21)
  mPORTBSetBits(BIT_9); // CS active low
  PPSOutput(2, RPB5, SDO2); // use RPB5 (pin 14) for SDO2
  PPSInput(3, SDI2,RPA4); // SDI2
  
  SpiChnOpen(pe_spi, SPI_OPEN_ON | SPI_OPEN_MODE8 | SPI_OPEN_MSTEN | SPI_OPEN_CKE_REV, spiClkDiv);
  
  writePE(IOCON, ( CLEAR_BANK   | CLEAR_MIRROR | SET_SEQOP |
                   CLEAR_DISSLW | CLEAR_HAEN   | CLEAR_ODR |
                   CLEAR_INTPOL ));
}

void clearBits(unsigned char addr, unsigned char bitmask){
  if (addr <= 0x15){
    unsigned char cur_val = readPE(addr);
    writePE(addr, cur_val & (~bitmask));
  }
}

void setBits(unsigned char addr, unsigned char bitmask){
  if (addr <= 0x15){
    unsigned char cur_val = readPE(addr);
    writePE(addr, cur_val | (bitmask));
  }
}

void toggleBits(unsigned char addr, unsigned char bitmask){
  if (addr <= 0x15){
    unsigned char cur_val = readPE(addr);
    writePE(addr, cur_val ^ (bitmask));
  }
}

unsigned char readBits(unsigned char addr, unsigned char bitmask){
  if (addr <= 0x15){
    unsigned char cur_val = readPE(addr) & bitmask ;
    return cur_val ;
  }
}

void mPortYSetPinsOut(unsigned char bitmask){
  clearBits(IODIRY, bitmask);
}

void mPortZSetPinsOut(unsigned char bitmask){
  clearBits(IODIRZ, bitmask);
}

void mPortYSetPinsIn(unsigned char bitmask){
  setBits(IODIRY, bitmask);
}

void mPortZSetPinsIn(unsigned char bitmask){
  setBits(IODIRZ, bitmask);
}

void mPortYIntEnable(unsigned char bitmask){
  setBits(GPINTENY, bitmask);
}

void mPortZIntEnable(unsigned char bitmask){
  setBits(GPINTENZ, bitmask);
}

void mPortYIntDisable(unsigned char bitmask){
  clearBits(GPINTENY, bitmask);
}

void mPortZIntDisable(unsigned char bitmask){
  clearBits(GPINTENZ, bitmask);
}

void mPortYEnablePullUp(unsigned char bitmask){
  setBits(GPPUY, bitmask);
}

void mPortZEnablePullUp(unsigned char bitmask){
  setBits(GPPUZ, bitmask);
}

void mPortYDisablePullUp(unsigned char bitmask){
  clearBits(GPPUY, bitmask);
}

void mPortZDisablePullUp(unsigned char bitmask){
  clearBits(GPPUZ, bitmask);
}

inline void writePE(unsigned char reg_addr, unsigned char data) {
  unsigned char junk = 0;
   
  // test for ready
  while (TxBufFullSPI2());
  
  // CS low to start transaction
  CLEAR_CS
  // 8-bits
  SPI_Mode8();
  // OPCODE and HW Address (Should always be 0b0100000), set LSB for write
  WriteSPI2((PE_OPCODE_HEADER | WRITE));
  // test for done
  while (SPI2STATbits.SPIBUSY); // wait for byte to be sent
  junk = ReadSPI2();
  // Input Register Address

  WriteSPI2(reg_addr);
  while (SPI2STATbits.SPIBUSY); // wait for byte to be sent
  junk = ReadSPI2();
  // One byte of data to write to register
  
  WriteSPI2(data);
  // test for done
  while (SPI2STATbits.SPIBUSY); // wait for end of transaction
  junk = ReadSPI2();
  // CS high
  SET_CS
  
}

inline unsigned char readPE(unsigned char reg_addr) {
  unsigned char out = 0;
  
  // test for ready
  while (TxBufFullSPI2());
  
  // CS low to start transaction
  CLEAR_CS
  
  // 8-bits
  SPI_Mode8();
  // OPCODE and HW Address (Should always be 0b0100000), clear LSB for write
  WriteSPI2((PE_OPCODE_HEADER | READ));
  // test for done
  while (SPI2STATbits.SPIBUSY); // wait for byte to be sent
  out = ReadSPI2(); //junk
  // Input Register Address
  // test for ready
  while (TxBufFullSPI2());
  // 8-bits
  
  WriteSPI2(reg_addr);
  while (SPI2STATbits.SPIBUSY); // wait for byte to be sent
  out = ReadSPI2(); // junk
  // One byte of dummy data to write to register
  // test for ready
  while (TxBufFullSPI2());
  // 8-bits
  
  WriteSPI2(out);
  // test for done
  while (SPI2STATbits.SPIBUSY); // wait for end of transaction
  out = ReadSPI2(); // bingo
  // CS high
  SET_CS
  
  return out;
}
