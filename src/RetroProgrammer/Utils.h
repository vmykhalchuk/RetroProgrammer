#ifndef Utils_h
#define Utils_h

  #include <Arduino.h>
  #include <SPI.h>
  #include <SD.h>
  #include "LoggerA.h"
  #include "AVRConstants.h"

  // SD Card Module pins:
  // Arduino | Target
  //   Nano  |  MCU
  //------------------
  //    11   | MOSI
  //    12   | MISO
  //    13   | CLK
  //    4    | CS
  //    10   | is not used but always set to OUTPUT (read more in SD method implementation)
  boolean initSDCard();  

  //BUFFER SIZE FOR MCU MODEL CHAR STRING
  // 'TT_' + '112233445566' + '\0' - here TT is 'MN','R1', 'R2', 'ID', and 112233445566 - is a maximum length for 'ID' case; (see TargetProgramDetector#getTargetIdName)
  //          // 3 chars for type (ID_, R1_, R2_, MN_), 12 chars as a max length for ID - 6 bytes, 1 char '/0'
  const byte PROG_ID_BUFFER_SIZE = (3+12+1);
  // BUFFER SIZE FOR FILE PATH TO HRP FILE
  const byte FILE_PATH_BUFFER_SIZE = (8+1+3+1);

  const byte MCU_MODEL_HUMAN_NAME_BUFFER_SIZE = 12; // see getAVRModelNameById() for a longest string
  // can be one of (see getAVRModelIdByName() below)
  //  AVR@DDD - where DDD - model ID
  //  AVR[HHHHHH] - where HHHHHH - AVR signature
  //  AVR-NAME    - where NAME - is an human readable name (ATmega328P or ATtiny48, etc)
  const byte MCU_MODEL_BUFFER_SIZE = (4+MCU_MODEL_HUMAN_NAME_BUFFER_SIZE);

  #define ERROR_TARGET_DETECTOR 0x10

  #define initStatus() statusRes=0;
  #define checkStatus() if (statusRes != 0) return;
  #define checkStatusV(v) if (statusRes != 0) return v;
  #define checkOverrideStatus(err) if (statusRes != 0) { logDebugB("@pr@",statusRes); statusRes = err; return; }
  #define checkOverrideStatusV(err, v) if (statusRes != 0) { logDebugB("@pr@",statusRes); statusRes = err; return v; }
  #define returnStatus(err) { statusRes = err; return; }
  #define returnStatusV(err, v) { statusRes = err; return v; }

  int freeRam();
  #define logFreeRam() logDebugD("$$$RAM:",freeRam())

  /*
   * Returns Char (0..9,A..F) based on 4 bits of given 'b', either low or high 4 bits are used.
   * high4bits = true - then high 4 bits are used, otherwise low 4 bits
   */
  char convertByteToHexChar(byte b, boolean high4bits);

  // c - character to be converted to 4bits value
  byte convertHexCharToByte(byte c, byte& statusRes);
  
  inline byte convertTwoHexCharsToByte(const char* str, byte& statusRes) {
    byte b1 = convertHexCharToByte(str[0], statusRes); checkStatusV(0);
    byte b2 = convertHexCharToByte(str[1], statusRes); checkStatusV(0);
    return (b1 << 4) | b2;
  }

  inline int convert3DigitsToInt(const char* str, byte& statusRes) {
    int b1 = convertHexCharToByte(str[0], statusRes); checkStatusV(0);
    int b2 = convertHexCharToByte(str[1], statusRes); checkStatusV(0);
    int b3 = convertHexCharToByte(str[2], statusRes); checkStatusV(0);
    if (b1 > 9 || b2 > 9 || b3 > 9) {
      returnStatusV(0xFF, 0);
    }
    return b1 * 100 + b2 * 10 + b3;
  }

  boolean isWhiteChar(char c);

  // eol is not valid, only a..z, A..Z, 0..9, ".-_" are valid!
  boolean isValidNameChar(char c);

  inline int strLength(const char* str) {
    int r = 0;
    while (str[r] != 0) {
      r++;
    }
    return r;
  };


  //////////////////////////
  //   AVR Related
  //////////////////////////

  // return model ID by signature
  //        byte signBytes[3];
  //        readSignatureBytes(signBytes, statusRes); checkStatus();
  //        byte modelId = getAVRModelIdBySignature(signBytes, statusRes); checkStatus();
  inline byte getAVRModelIdBySignature(byte* signBytes, byte& statusRes) {
    initStatus();
    for (byte i = 0; i < MCU_AVR_TYPES_COUNT; i++) {
      if (signBytes[0] == MCU_AVR_TYPES[i][0] && 
          signBytes[1] == MCU_AVR_TYPES[i][1] &&
          signBytes[2] == MCU_AVR_TYPES[i][2]) {
        return i + 1;
      }
    }
    returnStatusV(0x17, 0);
  }

  // return modelId and memory configuration of this MCU
  inline byte getAVRModelAndConf(byte* signBytes, byte& flashPageSize, byte& flashPagesCount, byte& eepromPageSize, byte& eepromPagesCount, byte& statusRes) {
    byte modelId = getAVRModelIdBySignature(signBytes, statusRes); checkStatusV(0);
    flashPageSize = MCU_AVR_CONFIGS[modelId - 1][0];
    flashPagesCount = MCU_AVR_CONFIGS[modelId - 1][1];
    eepromPageSize = MCU_AVR_CONFIGS[modelId - 1][2];
    eepromPagesCount = MCU_AVR_CONFIGS[modelId - 1][3];
    return modelId;
  }
  
  // fills in buffer with modelName, based on modelId (human readable name is returned, e.g. ATmega328P, etc)
  // char mcuModel[MCU_MODEL_HUMAN_NAME_BUFFER_SIZE]; - string for mcuModel
  void getAVRModelNameById(char* modelName, byte modelId, byte& statusRes);
  
  // returns AVR Model ID based on modelStr in one of three formats:
  //  AVR@DDD - where DDD - model ID
  //  AVR[HHHHHH] - where HHHHHH - AVR signature
  //  AVR-NAME    - where NAME - is an human readable name (ATmega328P or ATtiny48, etc)
  //
  //  char mcuModel[MCU_MODEL_BUFFER_SIZE]; - string with mcuModel
  byte getAVRModelIdByName(const char* mcuModel, byte& statusRes);



  //////////////////////////
  //   SD Files Related
  //////////////////////////


  // Print 3 digits
  void fPrint3Dig(File& f, byte b);
  //  Print String + Byte + EOL
  void fPrintBln(File& f, String str, byte b);
  // Print Byte
  void fPrintB(File& f, byte b);

  byte read3DigByte(File& f, byte& statusRes);
  byte readHexByte(File& f, byte& statusRes);
  // remember to check isEOL before statusRes!!! When EOL, statusRes is also an error!
  byte readHexByteOrEOL(File& f, boolean& isEOL, byte& statusRes);
  boolean readChar(File& f, byte& c);
  int readToTheEOL(File& f, byte& statusRes);

#endif
