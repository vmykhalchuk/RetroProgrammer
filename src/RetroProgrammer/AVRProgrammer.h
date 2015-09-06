/*
 * Author Volodymyr Mykhalchuk (vmykhalchuk@gmail.com)
 * 
 * 1) Always call AVRProgrammer::setup() method from setup() method of your sketch.
 *    NOTE: Make it a first call, to setup MCU ports properly
 *    
 */
#ifndef AVRProgrammer_h
#define AVRProgrammer_h

  #include <Arduino.h>
  #include "Statuses.h"
  #include "LoggerA.h"
  #include "AVRConstants.h"

  class AVRProgrammer_Test;

  class AVRProgrammer
  {
    public:

    friend AVRProgrammer_Test;
    static void __translateErrorsToDisplayErrorCode(byte err, byte& mainErrCode, byte& subErrCode, byte& okCode);

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
    static void loadEEPROMMemoryPageByte(byte addr, byte b, byte& statusRes);
    /**
       addrMsb - 0000 00aa
       addrLsb - aaaa aa00
    */
    static void writeEEPROMMemoryPage(byte addrMsb, byte addrLsb, byte& statusRes);

    private:

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
    static const byte pinMosi =  5;
    static const byte pinMiso =  6;
    static const byte pinSck =  7;
    static const byte pinReset =  8;
    static const byte pinVccEnable =  3;
    static const byte pinAVccEnable =  9;


    static boolean targetMcuProgMode; // If true - Target MCU is in Programming mode
    static boolean targetMcuOutOfSync; // If true - Target MCU Programming mode is out of sync due to communication error, etc

    static void issueByteWriteCmd(byte b1, byte b2, byte b3, byte b4, byte& statusRes);

    inline static byte issueByteReadCmd2(byte b1, byte b2, byte& statusRes) { return issueByteReadCmd4(b1,b2,0x00,0x00,statusRes); }
    inline static byte issueByteReadCmd3(byte b1, byte b2, byte b3, byte& statusRes) { return issueByteReadCmd4(b1,b2,b3,0x00,statusRes); }
    static byte issueByteReadCmd4(byte b1, byte b2, byte b3, byte b4, byte& statusRes);
  
    inline static byte readProgramMemoryByte(byte highB, byte addrHigh, byte addrLow, byte& statusRes) { return issueByteReadCmd3((highB ? 0x28 : 0x20), addrHigh, addrLow, statusRes); }
    inline static byte readEepromMemoryByte(byte addrHigh, byte addrLow, byte& statusRes) { return issueByteReadCmd3(0xA0, addrHigh, addrLow, statusRes); }

    static byte sendReadByte(byte byteToSend);
  };


  class AVRProgrammer_Test {
    public:
    inline static byte _testReadProgramMemoryByte(byte highB, byte addrHigh, byte addrLow, byte& statusRes) { return AVRProgrammer::readProgramMemoryByte(highB, addrHigh, addrLow, statusRes); }
  };
  
#endif
