#ifndef Utils_h
#define Utils_h

  #include <Arduino.h>
  #include <SPI.h>
  #include <SD.h>
  #include "Statuses.h"
  #include "LoggerA.h"
  #include "AVRConstants.h"

class Utils {
  public:
  static void __translateErrorsToDisplayErrorCode(byte err, byte& mainErrCode, byte& subErrCode, byte& okCode);
};

  // BUFFER SIZE FOR FILE PATH TO HRP FILE (Program File)
  const byte FILE_PATH_BUFFER_SIZE = (8+1+3+1);

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

class UtilsAVR {
  public:

  static const byte MCU_MODEL_HUMAN_NAME_BUFFER_SIZE = 12; // see getAVRModelNameById() for a longest string
  // can be one of (see getAVRModelIdByName() below)
  //  AVR@DDD - where DDD - model ID
  //  AVR[HHHHHH] - where HHHHHH - AVR signature
  //  AVR-NAME    - where NAME - is an human readable name (ATmega328P or ATtiny48, etc)
  static const byte MCU_MODEL_BUFFER_SIZE = (4+MCU_MODEL_HUMAN_NAME_BUFFER_SIZE);

  // return model ID by signature
  //        byte signBytes[3];
  //        readSignatureBytes(signBytes, statusRes); checkStatus();
  //        byte modelId = getAVRModelIdBySignature(signBytes, statusRes); checkStatus();
  static byte getAVRModelIdBySignature(byte* signBytes, byte& statusRes);

  // return modelId and memory configuration of this MCU
  static byte getAVRModelAndConf(byte* signBytes, byte& flashPageSize, byte& flashPagesCount, byte& eepromPageSize, 
                                  byte& eepromPagesCount, byte& statusRes);
  
  // fills in buffer with modelName, based on modelId (human readable name is returned, e.g. ATmega328P, etc)
  // char mcuModel[MCU_MODEL_HUMAN_NAME_BUFFER_SIZE]; - string for mcuModel
  static void getAVRModelNameById(char* modelName, byte modelId, byte& statusRes);
  
  // returns AVR Model ID based on modelStr in one of three formats:
  //  AVR@DDD - where DDD - model ID
  //  AVR[HHHHHH] - where HHHHHH - AVR signature
  //  AVR-NAME    - where NAME - is an human readable name (ATmega328P or ATtiny48, etc)
  //
  //  char mcuModel[MCU_MODEL_BUFFER_SIZE]; - string with mcuModel
  static byte getAVRModelIdByName(const char* mcuModel, byte& statusRes);
};


  //////////////////////////
  //   SD Files Related
  //////////////////////////

class UtilsSD {
  public:

  static boolean initSDCard();

  // Write 3 digits
  static void fPrint3Dig(File& f, byte b);
  //  Write String + Byte + EOL
  static void fPrintBln(File& f, String str, byte b);
  // Write Byte as HEX
  static void fPrintB(File& f, byte b);

  static byte read3DigByte(File& f, byte& statusRes);
  static byte readHexByte(File& f, byte& statusRes);
  // remember to check isEOL before statusRes!!! When EOL, statusRes is also an error!
  static byte readHexByteOrEOL(File& f, boolean& isEOL, byte& statusRes);
  static boolean readChar(File& f, byte& c);
  static int readToTheEOL(File& f, byte& statusRes);
  
};

class UtilsTests {
  public:

    static boolean assertStringMatches(const char* str1, const char* str2, boolean debugMode);

  private:

    static void _testStringMatches_LogDebug(char c1, char c2);
  
};

#endif
