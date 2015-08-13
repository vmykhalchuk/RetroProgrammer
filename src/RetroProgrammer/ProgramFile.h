/*
 * User is responsible for SD and File lifecycle control!
 * 
 * initSDCard();
 */

#ifndef ProgramFile_h
#define ProgramFile_h

  #include <Arduino.h>
  #include "LoggerA.h"
  #include "Utils.h"
  #include <SPI.h>
  #include <SD.h>
  #include "AVRProgrammingRoutines.h"

  ////////////////////////////////////////////////
  // File related procedures!!!
  ////////////////////////////////////////////////

  // !!! Requires SD Card to be initialized by Utils initializeSDCard() method!

  ////////////////////////////////////////////////
  // Public declarations
  ////////////////////////////////////////////////

  void backupMcuData(String filePref, byte& statusRes);
  
  void backupMcuDataToFile(String fileName, byte progMemPageSize, byte progMemPagesCount, 
                        byte eepromMemPageSize, byte eepromMemPagesCount, byte& statusRes);
  
  void uploadMcuDataFromFile(String fileName, byte* targetMcuSign, byte progMemPagesCount, byte progMemPageSize,
                        byte eepromMemPagesCount, byte eepromMemPageSize, byte& statusRes);
  
  void uploadMcuDataFromFile(String fileName, byte targetMcuModel, byte& statusRes);


  ////////////////////////////////////////////////
  // Private declarations (constants)
  ////////////////////////////////////////////////

  // For parsing the file
  const byte LINE_TYPE_COMMENT = 0x01; // Comment
  const byte LINE_TYPE_TYP     = 0x10; // Type ("AVR","PIC",...)
  const byte LINE_TYPE_MDL     = 0x11; // Model (1,2,3,4,....)
  const byte LINE_TYPE_SGN     = 0x12; // Signature (for AVR - 3 bytes)
  const byte LINE_TYPE_LKB     = 0x20; // Lock Bits
  const byte LINE_TYPE_FSB     = 0x21; // Fuse Bits
  const byte LINE_TYPE_FHB     = 0x22; // Fuse High Bits
  const byte LINE_TYPE_EFB     = 0x23; // Extended Fuse Bits
  const byte LINE_TYPE_CLB     = 0x24; // Calibration Byte
  const byte LINE_TYPE_PRM     = 0x30; // Flash/Program Memory Page
  const byte LINE_TYPE_ERM     = 0x31; // EEPROM Page

  const byte LINE_READ_BUFFER_SIZE = 128;

  void _testProgramming(byte& statusRes);

#endif
