#include "LoggerA.h"
#include "Statuses.h"
#include "Utils.h"
#include "AVRConstants.h"
#include "AVRProgrammer.h"
#include "ConfFile.h"
#include "ProgramFile.h"
#include "TargetProgramDetector.h"
#include "HWInterface.h"
#include "Tests.h"

#include <SPI.h>
#include <SD.h>

#define ONEWIRE_CRC8_TABLE 0
#include <OneWire.h>

// For Random read
//------------------
//    A7
const byte pinRandomRead = A7;


#define RAM_DEPOSIT_SIZE 55
byte RAM_DEPOSIT[RAM_DEPOSIT_SIZE];

void setup() {
  #if RAM_DEPOSIT_SIZE > 0
    RAM_DEPOSIT[0]=0;
  #endif

  logDebug("Hello World!");

  AVRProgrammer::setup();
  HWInterface::setup();
  //                           TARGET_MCU_ID__W0, TARGET_MCU_ID__W1, TARGET_MCU_ID__W2, TARGET_MCU_ID__MANUAL0, TARGET_MCU_ID__MANUAL1
  TargetProgramDetector::setup(       A0        ,       A1         ,      A2          ,       A3              ,         A6            );
  Logger_setup();

  //setup_test();
  setup_prod();
}

void setup_test() {//Used for testing
  byte statusRes=0;
  
  logFreeRam();

  testUtilsGen();
  testUtilsAVR();
  
  #if 0 // Test AVR Signature Read (Target MCU must be connected)
    Tests_AVRProgrammer::testAVRSignatureRead();
  #endif
  
  #if 0
    Tests_TargetProgramDetector::testTargetProgramDetector();
  #endif

  #if 0 // Upoad test files to SD Card!!!
    // Init SD Card
    if (!UtilsSD::initSDCard()) {
      logError("SD init failed!");
      return;
    }
    Tests_ConfFile::testConfFile();
  #endif

  #if 0 // Test Board ports directly
    // select only one!
    Tests_Board::testSDCardPorts();
    Tests_Board::testManualProgramSelectorPorts();
  #endif

  #if 0 // Test HWInterface
    Tests_HWInterface::testLedsAndBtns();
  #endif

  #if 0 // Test Display Error
    displayError(2, 4, 0);
  #endif

  logInfo("Happy testing day!");
  logFreeRam();
}

void displayError(byte mainErr, byte subErr, byte okNumber) {
  // Blink ERR mainErr times
  // Blink AUTO subErr times
  //
  // 1 - No Target device discovered => make sure is properly connected
  // 2 - SD Card failure => Make sure SD Card is attached and proper files are uploaded
  // 2-0 - Not detected => Insert SD Card
  // 2-1 - No Conf file => Make sure Conf file is available on SD card
  // 2-2 - Corrupted Conf file => Check structure, OK display detail of error:
  //           1 - corrupted file, wrong EOL, [0D 0A] sequence is not followed!
  //           2 - unexpected EOL or EOF
  //           3 - too long MCU_MODEL in conf file!
  //           4 - too long FILE_PATH in conf file!
  // 2-3 - Corrupted Program file => Fix it or upload new
  // 2-4 - No space for backups => Remove some backup files to free up space
  // 2-5 - Failure writing Conf file => Make sure SD Card is not corrupted
  // 2-6 - Failure writing Program file => Make sure SD Card is not corrupted
  // 2-7 - Failure opening Program file => Make sure SD Card is not corrupted
  // 2-A - Failure reading file - file format is corrupted
  // 3 - Auto Program Identification Failed => Make sure Target ICSP+TID connector compatible, and wires are not too long (no noise on wires)
  // 3-1 - W1 or W1 wrong ADC value(s)
  // 3-2 - W0 - 1wire CRC didn't match
  // 3-3 -       >>>>
  // 3-4 - failure on the line, too much noise
  // 3-5 -       >>>>
  // 3-6 - W0 - strange state of 1wire device
  // 3-7 - wrong resistive divider value (line unknown)
  // 3-9 - W0 wrong ADC value
  // 3-A - wrong resistive divider value W0
  // 3-B - wrong resistive divider value W1
  // 3-C - wrong resistive divider value W2
  // 4 - No program file found => Make sure file is available on SD Card & Configuration file is updated with TargetID
  // 5 - Wrong Target MCU model => Make sure Conf/Program file is correct and Target MCU is correct model
  // 6 - Bad program file => Fix it or upload new
  // 7 - VERIFY: Program file & Target MCU didn't match => See AUTO for details
  // 7-1 - Program EEPROM differs => See OK led for a segment where difference starts
  // 7-2 - Flash EEPROM differs => See OK led for a segment where difference starts
  // 7-3 - Fuse/Lock bits differs => See OK led for which byte differs
  // 8 - Target MCU communication failure => Make sure cable is not too long, etc
  // A - General System Failure => Logging required
  // A-1 - Manual Program selector failure
          // ERR(0x68)
  // A-2 - ProgramFile routines error
          // ERR(0x50)
  // A-3 - AVRProgrammer routines error
  // A-4 - Utils routines error
  // A-5 - HWInterface routines error
  // A-6 - ConfFile routines error
  // A-7 - TargetProgramDetector routines error
  // A-0 - Unknown

  while (true) {
    for (int i = 0; i < 20; i++) {
      if (HWInterface::readButtons()) {
        if (i < subErr) {
          HWInterface::setLedOnOff(HWInterface::LED_AUTO, true);
        }
        if (i < okNumber) {
          HWInterface::setLedOnOff(HWInterface::LED_OK, true);
        }
      }
      if (i < mainErr) {
        HWInterface::setLedOnOff(HWInterface::LED_ERR, true);
      }
      
      HWInterface::runLeds(15); // 300ms
      
      HWInterface::setLedOnOff(HWInterface::LED_ERR, false);
      HWInterface::setLedOnOff(HWInterface::LED_AUTO, false);
      HWInterface::setLedOnOff(HWInterface::LED_OK, false);
      
      HWInterface::runLeds(15); // 300ms
    }
  }
}

void confirmProgramSelected(byte btn) {
  byte no = 0;
  if (btn == HWInterface::BTN_UPLOAD) {
    no = 3;
  } else if (btn == HWInterface::BTN_VERIFY) {
    no = 2;
  } else if (btn == HWInterface::BTN_BACKUP) {
    no = 1;
  }
  for (int i = 0; i < no; i++) {
    HWInterface::setLedOnOff(HWInterface::LED_OK, true);
    HWInterface::runLeds(20); // 400ms
    HWInterface::setLedOnOff(HWInterface::LED_OK, false);
    HWInterface::runLeds(20); // 400ms
  }
}

void setup_prod() {
  byte statusRes=0;
  // Init SD Card
  if (!UtilsSD::initSDCard()) {
    logError("SD init failed!");
    displayError(0x2, 0x0, 0x0);
    return;
  }
  
  delay(500);
  HWInterface::setLedOnOff(HWInterface::LED_RDY, true);
  byte btn = HWInterface::waitForUserCommand();
  HWInterface::setLedOnOff(HWInterface::LED_RDY, false);
  logInfoD("Command:",btn);
  
  // find out what program is selected
  // 3 chars for type (ID_, R1_, R2_, MN_), 12 chars as a max length for ID - 6 bytes, 1 char '/0'
  char progIdBuf[TargetProgramDetector::PROG_ID_BUFFER_SIZE];
  boolean autoSelected;
  TargetProgramDetector::getProgId(progIdBuf, autoSelected, statusRes);
  if (statusRes > 0) {
    logError("getProgId");
    byte mainErrCode, subErrCode, okCode;
    TargetProgramDetector::__translateErrorsToDisplayErrorCode(statusRes, mainErrCode, subErrCode, okCode);
    displayError(mainErrCode, subErrCode, okCode);
    return;
  } else {
    logInfoS("ProgId:", progIdBuf);
  }
  delay(500);
  confirmProgramSelected(btn);
  delay(500);

  // Prepare Target MCU for Programming
  AVRProgrammer::startupTargetMcuProgramming(statusRes);
  if (statusRes > 0) {
    logError("ProgEn failed!");
    AVRProgrammer::shutdownTargetMcu();
    displayError(0x1, 0x0, 0x0);
    return;
  } else {
    logInfo("Started prog!");
  }
  delay(200);

  if (btn == HWInterface::BTN_BACKUP) {
    logInfo("Backup to file...");
    ProgramFile::backupMcuData("BACK", progIdBuf, statusRes);
    if (statusRes > 0) {
      logErrorB("Error backing-up!", statusRes);
      byte mainErrCode, subErrCode, okCode;
      ProgramFile::__translateErrorsToDisplayErrorCode(statusRes, mainErrCode, subErrCode, okCode);
      displayError(mainErrCode, subErrCode, okCode);
      return;
    }
    logInfo("Success backing-up!");
  } else if (btn == HWInterface::BTN_UPLOAD) {
  }

#if 0
  byte signBytes[3];
  #if 1
  // make sure it is ATmega328P
  AVRProgrammer::readSignatureBytes(signBytes,statusRes);
  if (statusRes > 0) {
    logError("signature error!");
    return;
  }
  delay(2000);
  #endif

  #if 1
  logInfo("Uploading...");
  ProgramFile::uploadMcuDataFromFile("TEST01", signBytes,
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_64, 
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_4, statusRes);
  if (statusRes > 0) {
    logError("Error uploading!");
    return;
  }
  logInfo("Success uploading!");
  delay(2000);
  #endif

  #if 1
  // test example of programming
  ProgramFile_Test::_testProgramming(statusRes);
  if (statusRes > 0) { logErrorB("Failed!", statusRes); return; }
  logInfo("Success testing!");
  delay(2000);
  #endif
  
  #if 0
  logInfo("Reading...");
  ProgramFile::backupMcuDataToFile("RND" + String(analogRead(pinRandomRead),HEX), 
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_64, 
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_4, statusRes);
  if (statusRes > 0) {
    logError("Error reading!");
    return;
  }
  logInfo("Success reading!");
  delay(2000);
  #endif
#endif

  // now shutdown Target MCU
  AVRProgrammer::shutdownTargetMcu();

  logInfo("Done!");
  HWInterface::setLedOnOff(HWInterface::LED_OK, true);
  while (true) {
    HWInterface::runLeds(1);
  }
}

void loop()
{
}
