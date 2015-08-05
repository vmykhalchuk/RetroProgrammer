/*
 * User is responsible for SD and File lifecycle control!
 * 
 * initConfFileSD();
 * boolean openConfFile();
 * boolean openConfFile(const char* fname);
 * void closeConfFile();
 */

#ifndef ConfFile_h
#define ConfFile_h

  #include <Arduino.h>
  #include "LoggerA.h"
  #include "Utils.h"
  #include <SPI.h>
  #include <SD.h>


  ////////////////////////////////////////////////
  // File related procedures!!!
  ////////////////////////////////////////////////

  // SD Card Module pins:
  // Arduino | Target
  //   Nano  |  MCU
  //------------------
  //    11   | MOSI
  //    12   | MISO
  //    13   | CLK
  //    4    | CS
  //    10   | is not used but always set to OUTPUT (read more in SD method implementation)
  boolean initConfFileSD();
  boolean openConfFile();
  boolean openConfFile(const char* fname);
  void closeConfFile();

  ////////////////////////////////////////////////
  // MAIN/PUBLIC FUNCTIONS
  ////////////////////////////////////////////////

  /*
   * mcuModelId - see AVRConstants.h
   * char mcuModelBuf[MCU_MODEL_BUFFER_SIZE]; - buffer to store MCU Model (note - it will be actively used while method is being called!
   * char filePath[FILE_PATH_BUFFER_SIZE]; - if length is 0 - then no match detected! // at the moment we support only filename and no directory path (to save space)
   * returns false if no configuration line matched (progId and mcuModelId must match)
   */
  boolean getFilePathByProgIdAndMcuModel(const char* progId, byte mcuModelId, char* mcuModelBuf, char* filePath, byte& statusRes);

  /*
   * ---- OBSOLETTE ----
   * char dir1Name[8+4+1]; // if ends with ".___" - then also dir2Name is defined
   * char dir2Name[8+1];
   * boolean matchDetected; // false if dirNames were autogenerated, true if loaded from conf file
   * getDirNameByProgId("ID_XXXXYYYYYYYY", dir1Name, dir2Name, matchDetected, statusRes);
   */
  void getDirNameByProgId(const char* progId, char* dir1Name, char* dir2Name, boolean& matchDetected, byte& statusRes);
  void autogenerateDirNamesByProgId(const char* progId, char* dir1Name, char* dir2Name);

  /*
   * Scans through config and searches for progId match, returns dirName if one detected.
   * Search is continued from last opened point, so consecutive calls to this method will return every next occurence.
   * 
   * char mcuModel[MCU_MODEL_BUFFER_SIZE];
   * char filePath[FILE_PATH_BUFFER_SIZE]; - if length is 0 - then no match detected! // at the moment we support only filename and no directory path (to save space)
   * readRootConfFile("ID_XXXXXXXXXXX", dirName, matchDetected, statusRes);
   */
  void readRootConfFile(const char* progId, char* mcuModel, char* filePath, byte& statusRes);

  ////////////////////////////////////////////////
  // PRIVATE FUNCTIONS
  ////////////////////////////////////////////////

  //boolean confFileAvailable();

  // skips all white spaces and comments, positions on the first valid character to read (if available)
  boolean skipToValidData(byte& statusRes);

  // skips to the next line, positions on first character of next line (if available)
  void skipLine(byte& statusRes);

  // returns false if there are no more characters to read, true otherwise
  void skipWhiteCharactersExpectNoEolNoEof(byte& statusRes); // skip all white characters (except EOL), positions on the first non-white character. Expect no EOL or EOF!

  // returns false if there are no more characters to read, true otherwise
  boolean skipWhiteCharacters(byte& statusRes); // skip all white characters (including EOL), positions on the first non-white character

  // reads char and checks for eol, however eof is not expected and is returned as error
  char readCharSafe(boolean& eol, byte& statusRes);

  ////////////////////////////////////////////////
  // Test hooks (used by Tests.h)
  ////////////////////////////////////////////////

  boolean __test_confFileOpen();
  boolean __test_confFileAvailable();
  char __test_confFileReadChar();
#endif
