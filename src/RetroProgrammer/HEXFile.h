/*
 * User is responsible for SD and File lifecycle control!
 * 
 * !!!! Always make sure that SD Card is initialized before hand!
 * initSDCard();
 */

#ifndef HEXFile_h
#define HEXFile_h

  #include <Arduino.h>
  #include "Statuses.h"
  #include "LoggerA.h"
  #include "Utils.h"
  #include "AVRProgrammer.h"
  #include <SPI.h>
  #include <SD.h>

  class HEXFile_TestStub;

  class HEXFile {
    public:

      friend HEXFile_TestStub;

      static void __translateErrorsToDisplayErrorCode(byte err, byte& mainErrCode, byte& subErrCode, byte& okCode);

      static void openFile(File& f, String fileName, int mode, byte& statusRes);

      static byte LINE_TYPE_DATA;
      static byte LINE_TYPE_EOF;

      // Every line is:
      // :LLAAAATTDDDD.....DDCC
      // LL - Length of Data
      // AAAA - Address
      // TT - Line Type (0x00 - Data; 0x01 - End Of File; ...)
      // NOTE: buffer must be around 16 bytes for average Intel HEX file, however could be up to 255. bufSize should be provided to tell what maximum line length could be loaded!
      static void readLine(File& f, byte& lineType, byte* buf, byte bufSize, byte& resSize, unsigned int& address, byte& statusRes);
      
      static void closeFile(File& f);

    private:

      static byte _crc8;
      static void crc8Init();
      static byte crc8Append(byte b);
      static byte crc8Close();
  };


  class HEXFile_TestStub {
    public:
      static void _testProgramming(byte& statusRes);
  };

#endif
