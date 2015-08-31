#ifndef Tests_h
#define Tests_h

  #include <Arduino.h>
  #include "LoggerA.h"
  
  #include "Utils.h"
  #include "AVRConstants.h"
  #include "TargetProgramDetector.h"
  #include "ConfFile.h"
  #include "HWInterface.h"

  // nothing is required to run these tests (no special connections just Arduino Nano/Micro)
  void testUtilsGen();
  void __testConvertByteToHexChar();
  void __testConvertHexCharToByte();
  void __testConvertTwoHexCharsToByte();
  void __testConvert3DigitsToInt();
  void __testIsWhiteChar();
  void __testIsValidNameChar();
  void __testStrLength();

  // nothing is required to run these tests (no special connections just Arduino Nano/Micro)
  void testUtilsAVR();
  void __testGetAVRModelIdBySignature();
  void __testGetAVRModelAndConf();
  void __testGetAVRModelNameById();
  void __testGetAVRModelIdByName();


  // requires A0,A1,A2,A3 and A4 connections, depending on type of test, see schematics
  void testTargetProgramDetector();
  void __testOneWireID(int d);
  void __testGetProgId(int d);
  

  class Tests_ConfFile {
    public:
      // 1) requires SD Card to be connected (see ConfFile.h for more details)
      // 2) requires test files to be written to SD card
      static void testConfFile();
      
    private:
      static char __testConfFileName[];
      static boolean _testOpenConfFile(byte pos);
      static void testAutogenerateDirNamesByProgId();
      static void testSkipToValidData();
      static void testReadRootConfFile();
      // doesn't support strings longer then 125 characters!
      //static boolean _testStringMatches(const char* str1, const char* str2, boolean debugMode);
  };

  /**
   * 1) Make sure that HWInterface::setup is called; and
   * 2) proper components are attached to MCU pins
   *      a) ports: D2, A4, A5 - see schematics
   */
  class Tests_HWInterface {
    public:
      static void testLedsAndBtns();
  };

#endif
