/*
 * Author Volodymyr Mykhalchuk (vmykhalchuk@gmail.com)
 * 
 * 1) Always call AVRProgrammingRoutines_setup() method from setup() method of your sketch.
 *    NOTE: Make it a first call, to setup MCU ports properly
 *    
 */
#ifndef AVRProgrammingRoutines_h
#define AVRProgrammingRoutines_h

  #include <Arduino.h>
  #include "LoggerA.h"
  #include "AVRConstants.h"
  #include "Utils.h"

  // Programing pins:
  // Arduino | Target
  //   Nano  |  MCU
  //------------------
  //     5   | MOSI
  //     6   | MISO
  //     7   | SCK
  //     8   | RESET
  //     3   | VCC Enable
  //     9   | AVCC Enable
  const byte pinMosi =  5;
  const byte pinMiso =  6;
  const byte pinSck =  7;
  const byte pinReset =  8;
  const byte pinVccEnable =  3;
  const byte pinAVccEnable =  9;


  /** Must always be called first and only once!
  */
  void AVRProgrammingRoutines_setup();

  void shutdownTargetMcu();
  void startupTargetMcuProgramming(byte& statusRes);

  void issueByteWriteCmd(byte b1, byte b2, byte b3, byte b4, byte& statusRes);

  #define issueByteReadCmd2(b1, b2, statusRes) issueByteReadCmd4(b1,b2,0x00,0x00,statusRes)
  #define issueByteReadCmd3(b1, b2, b3, statusRes) issueByteReadCmd4(b1,b2,b3,0x00,statusRes)
  byte issueByteReadCmd4(byte b1, byte b2, byte b3, byte b4, byte& statusRes);
  
  boolean isTargetMcuBusy(byte& statusRes);

  // byte signBytes[3];
  // readSignatureBytes(signBytes, statusRes); checkStatus();
  void readSignatureBytes(byte* signBytes, byte& statusRes);

  #define readLockBits(statusRes) issueByteReadCmd2(0x58,0x00,statusRes)
  #define readFuseBits(statusRes) issueByteReadCmd2(0x50,0x00,statusRes)
  #define readFuseHighBits(statusRes) issueByteReadCmd2(0x58,0x08,statusRes)
  #define readExtendedFuseBits(statusRes) issueByteReadCmd2(0x50,0x08,statusRes)
  #define readCalibrationByte(statusRes) issueByteReadCmd2(0x38,0x00,statusRes)
  #define readProgramMemoryByte(highB, addrHigh, addrLow, statusRes) issueByteReadCmd3((highB ? 0x28 : 0x20), addrHigh, addrLow, statusRes)
  #define readEepromMemoryByte(addrHigh, addrLow, statusRes) issueByteReadCmd3(0xA0, addrHigh, addrLow, statusRes)

  void readProgramMemoryPage(byte* pageBuffer, byte pageNumber, byte pageSize, byte& statusRes);
  void readEepromMemoryPage(byte* pageBuffer, byte pageNumber, byte pageSize, byte& statusRes);
  
  //boolean loadProgramMemoryPageWord(byte adrLSB, int w);
  #define loadProgramMemoryPageByte(highB, addrLsb, b, statusRes) issueByteWriteCmd((highB ? 0x48 : 0x40), 0x00, addrLsb, b, statusRes)
  /**
    addr - 0..3
  */
  #define loadEEPROMMemoryPageByte(addr, b, statusRes) { if (addr > 3) returnStatus(0xFF); issueByteWriteCmd(0xC1, 0x00, addr, b, statusRes); }
  #define writeProgramMemoryPage(addrMsb, addrLsb, statusRes) issueByteWriteCmd(0x4C, addrMsb, addrLsb, 0x00, statusRes)
  /**
     addrMsb - 0000 00aa
     addrLsb - aaaa aa00
  */
  #define writeEEPROMMemoryPage(addrMsb, addrLsb, statusRes) { if (addrMsb & B00000011 > 0) returnStatus(0xFF); \
        if (addrLsb & B11111100 > 0) returnStatus(0xFF); \
        issueByteWriteCmd(0xC2, addrMsb, addrLsb, 0x00, statusRes); }
  byte sendReadByte(byte byteToSend);

#endif
