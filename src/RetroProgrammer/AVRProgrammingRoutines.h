/*
 * Author Volodymyr Mykhalchuk (vmykhalchuk@gmail.com)
 * 
 * 1) Always call AVRProgrammer::setup() method from setup() method of your sketch.
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


  class AVRProgrammer
  {
    public:

    /** 
     *  Must always be called before creating any objects of this class
     */
    static void setup();
    
    static void startupTargetMcuProgramming(byte& statusRes);
    static void shutdownTargetMcu();
    static boolean isTargetMcuBusy(byte& statusRes);
    static void waitForTargetMCU(byte& statusRes);

    // byte signBytes[3];
    // readSignatureBytes(signBytes, statusRes); checkStatus();
    static void readSignatureBytes(byte* signBytes, byte& statusRes);

    inline static byte readLockBits(byte& statusRes) { return issueByteReadCmd2(0x58,0x00,statusRes); }
    inline static byte readFuseBits(byte& statusRes) { return issueByteReadCmd2(0x50,0x00,statusRes); }
    inline static byte readFuseHighBits(byte& statusRes) { return issueByteReadCmd2(0x58,0x08,statusRes); }
    inline static byte readExtendedFuseBits(byte& statusRes) { return issueByteReadCmd2(0x50,0x08,statusRes); }
    inline static byte readCalibrationByte(byte& statusRes) { return issueByteReadCmd2(0x38,0x00,statusRes); }
    
    static void readProgramMemoryPage(byte* pageBuffer, byte pageNumber, byte pageSize, byte& statusRes);
    static void readEepromMemoryPage(byte* pageBuffer, byte pageNumber, byte pageSize, byte& statusRes);
  
    //boolean loadProgramMemoryPageWord(byte adrLSB, int w);
    inline static void loadProgramMemoryPageByte(byte highB, byte addrLsb, byte b, byte& statusRes) { issueByteWriteCmd((highB ? 0x48 : 0x40), 0x00, addrLsb, b, statusRes); }
    inline static void writeProgramMemoryPage(byte addrMsb, byte addrLsb, byte& statusRes) { issueByteWriteCmd(0x4C, addrMsb, addrLsb, 0x00, statusRes); }
    
    /**
      addr - 0..3
    */
    inline static void loadEEPROMMemoryPageByte(byte addr, byte b, byte& statusRes) { if (addr > 3) returnStatus(0xFF); issueByteWriteCmd(0xC1, 0x00, addr, b, statusRes); }
    /**
       addrMsb - 0000 00aa
       addrLsb - aaaa aa00
    */
    inline static void writeEEPROMMemoryPage(byte addrMsb, byte addrLsb, byte& statusRes) { 
        if (addrMsb & B00000011 > 0) returnStatus(0xFF);
        if (addrLsb & B11111100 > 0) returnStatus(0xFF);
        issueByteWriteCmd(0xC2, addrMsb, addrLsb, 0x00, statusRes);
    }

    inline static byte _testReadProgramMemoryByte(byte highB, byte addrHigh, byte addrLow, byte& statusRes) { return readProgramMemoryByte(highB, addrHigh, addrLow, statusRes); }
    
    private:
    static void issueByteWriteCmd(byte b1, byte b2, byte b3, byte b4, byte& statusRes);

    inline static byte issueByteReadCmd2(byte b1, byte b2, byte& statusRes) { return issueByteReadCmd4(b1,b2,0x00,0x00,statusRes); }
    inline static byte issueByteReadCmd3(byte b1, byte b2, byte b3, byte& statusRes) { return issueByteReadCmd4(b1,b2,b3,0x00,statusRes); }
    static byte issueByteReadCmd4(byte b1, byte b2, byte b3, byte b4, byte& statusRes);
  
    inline static byte readProgramMemoryByte(byte highB, byte addrHigh, byte addrLow, byte& statusRes) { return issueByteReadCmd3((highB ? 0x28 : 0x20), addrHigh, addrLow, statusRes); }
    inline static byte readEepromMemoryByte(byte addrHigh, byte addrLow, byte& statusRes) { return issueByteReadCmd3(0xA0, addrHigh, addrLow, statusRes); }

    static byte sendReadByte(byte byteToSend);
  };
#endif
