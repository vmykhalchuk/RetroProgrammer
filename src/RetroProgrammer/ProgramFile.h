/*
 * User is responsible for SD and File lifecycle control!
 * 
 * !!!! Always make sure that SD Card is initialized before hand!
 * initSDCard();
 */

#ifndef ProgramFile_h
#define ProgramFile_h

  #include <Arduino.h>
  #include "Statuses.h"
  #include "LoggerA.h"
  #include "Utils.h"
  #include "AVRProgrammer.h"
  #include <SPI.h>
  #include <SD.h>

  class ProgramFile_Test;

  class ProgramFile {
    public:

      friend ProgramFile_Test;

      static void __translateErrorsToDisplayErrorCode(byte err, byte& mainErrCode, byte& subErrCode, byte& okCode);

      /**
       * filePref must be 4 chars max!
       * File is then generated with 4 additional digits to give a number of backup file.
       */
      static void backupMcuData(String filePref, const char* programId, byte& statusRes);
  
      static void uploadMcuDataFromFile(String fileName, byte* targetMcuSign, byte progMemPagesCount, byte progMemPageSize,
                        byte eepromMemPagesCount, byte eepromMemPageSize, byte& statusRes);
  
      static void uploadMcuDataFromFile(String fileName, byte targetMcuModel, byte& statusRes);

    private:  // Constants For parsing the file
      static const byte LINE_TYPE_COMMENT = 0x01; // Comment
      static const byte LINE_TYPE_TID     = 0x1F; // Target ID ("MN_A1C3",....)
      static const byte LINE_TYPE_TYP     = 0x10; // Type ("AVR","PIC",...)
      static const byte LINE_TYPE_MDL     = 0x11; // Model (1,2,3,4,....)
      static const byte LINE_TYPE_SGN     = 0x12; // Signature (for AVR - 3 bytes)
      static const byte LINE_TYPE_LKB     = 0x20; // Lock Bits
      static const byte LINE_TYPE_FSB     = 0x21; // Fuse Bits
      static const byte LINE_TYPE_FHB     = 0x22; // Fuse High Bits
      static const byte LINE_TYPE_EFB     = 0x23; // Extended Fuse Bits
      static const byte LINE_TYPE_CLB     = 0x24; // Calibration Byte
      static const byte LINE_TYPE_PRM     = 0x30; // Flash/Program Memory Page
      static const byte LINE_TYPE_ERM     = 0x31; // EEPROM Page

      static const byte LINE_READ_BUFFER_SIZE = 128;

    private:
      static void backupMcuDataToFile(const char* programId, byte progMemPageSize, byte progMemPagesCount, 
                        byte eepromMemPageSize, byte eepromMemPagesCount, byte& statusRes);

      static const byte line_types[11][4];
      static byte programBuffer[];

      static void __openFile(File& f, String fileName, int mode, byte& statusRes);
      static void openFile2(File& f, String fileName, int mode, byte& statusRes);
      static File backupFile;
      static void findAndOpenNextFileBackupFile(String filePref, byte& statusRes);

      static void uploadMcuDataFromFile_internal(boolean progMode, String fileName, byte targetMcuModel, byte& statusRes);
      static void uploadMcuDataFromFile_internal(boolean progMode, String fileName, byte* targetMcuSign,
                              byte progMemPagesCount, byte progMemPageSize,
                              byte eepromMemPagesCount, byte eepromMemPageSize, byte& statusRes);
      static void readLine(File& f, byte& lineType, byte* buffer, int& resSize, int& pageNo, byte& statusRes);
      static void uploadProgramMemoryPage(byte* buf, int pageNo, 
                              byte progMemPagesCount, byte progMemPageSize, byte& statusRes);
      
  };


  class ProgramFile_Test {
    public:
      static void _testProgramming(byte& statusRes);
  };

#endif
