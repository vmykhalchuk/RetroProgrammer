#include "LoggerA.h"
#include "Statuses.h"
#include "Utils.h"
#include "AVRConstants.h"
#include "AVRProgrammer.h"
#include "ConfFile.h"
#include "HEXFile.h"
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

  // TODO:
    // 1) try to upload again and backup and compare BACKUP3 with BACKUP4 (BACKUP4 will be new generated), make sure they match and its not a noise but algorithm issue
    // *) switch logging to 4-DEBUG
    // 2) run setup_tests::Test programming second page of 1blink; see if data is uploaded as expected
  //setup_test();
  setup_prod();
}

void setup_test() {//Used for testing
  byte statusRes=0;
  
  //logFreeRam();

  #if 0
    testUtilsGen();
    testUtilsAVR();
  #endif
  
  #if 0 // Test AVR Signature Read (Target MCU must be connected)
    Tests_AVRProgrammer::testAVRSignatureRead();
  #endif

  #if 0 // Test programming second page of 1blink
                           // Word N Byte Hight | Low:
                           //  W0H   W0L   W1H   W1L   W2H   W2L   W3H   W3L   W4H   W4L   W5H   W5L   W6H   W6L   W7H   W7L
    byte pageToUpload[128] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
                              0x10, 0x11, 0x00, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                              0x20, 0x21, 0x22, 0x23, 0x00, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
                              0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
                              0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
                              0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x01, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
                              0x20, 0x21, 0x22, 0x21, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
                              0x30, 0x31, 0x32, 0x31, 0x34, 0x32, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};
    Tests_AVRProgrammer::testUploadProgramTestPage(MCU_AVR_ATmega328P, 2, pageToUpload, sizeof(pageToUpload));
  #endif

  #if 0 // Test reading Intel HEX file
    Tests_HEXFile::testHEXFile05();
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

  #if 0 // Test Board ports directly (will lock for indefinite)
    // select only one!
    Tests_Board::testSDCardPorts();
    Tests_Board::testManualProgramSelectorPorts();
  #endif

  #if 0 // Test HWInterface (will lock for indefinite)
    Tests_HWInterface::testLedsAndBtns();
  #endif

  #if 0 // Test Display Error (will lock for indefinite)
    displayError(2, 4, 0);
  #endif

  #if 0 // DO NOT RUN THESE, MAKE SURE THEY ARE SAFE FIRST!
    someOldTests();
  #endif

  logInfo("Happy testing day!");
  logFreeRam();
}

void displayError(byte mainErr, byte subErr, byte okNumber) {
  // Blink ERR mainErr times
  // Blink AUTO subErr times
  //
  // 1 - No Target device discovered => make sure ICSP is properly connected
  // 2 - SD Card failure => Make sure SD Card is attached, properly formatted and proper files are uploaded
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
  // 3 - Auto Program Identification Failed => Make sure Target board is ICSP+TID compatible, and wires are not too long (no noise on wires)
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
  // 5 - Wrong Target MCU model => Make sure Conf/Program file is correct and Target MCU resembles expected MCU model
  // 6 - Corrupted Program file => Fix it or upload new
  // 7 - VERIFY: Program file & Target MCU didn't match => See AUTO for details (press and hold BACKUP button)
  // 7-1 - Program EEPROM differs => See OK led for a segment where difference starts
  // 7-2 - Flash EEPROM differs => See OK led for a segment where difference starts
  // 7-3 - Fuse/Lock bits differs => See OK led for which byte differs
  // 8 - Target MCU communication failure => Make sure cable is not too long, etc
  // A - General System Failure => See AUTO for mode details (press and hold BACKUP button)
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
  delay(500);
  
  // find out what program is selected
  // 3 chars for type (ID_, R1_, R2_, MN_), 12 chars as a max length for ID - 6 bytes, 1 char '/0'
  char progIdBuf[TargetProgramDetector::PROG_ID_BUFFER_SIZE];
  boolean autoSelected;
  TargetProgramDetector::getProgId(progIdBuf, autoSelected, statusRes);
  if (statusRes > 0) {
    logErrorB("getProgId",statusRes);
    byte mainErrCode, subErrCode, okCode;
    TargetProgramDetector::__translateErrorsToDisplayErrorCode(statusRes, mainErrCode, subErrCode, okCode);
    displayError(mainErrCode, subErrCode, okCode);
    return;
  }
  logInfoS("ProgId:", progIdBuf);
  if (autoSelected) {
    HWInterface::setLedOnOff(HWInterface::LED_AUTO, true);
  }
  
  HWInterface::runLeds(25); // 500ms
  confirmProgramSelected(btn);
  HWInterface::runLeds(50); // 1000ms

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
    
  } else if (btn == HWInterface::BTN_VERIFY) {
    logInfo("TESTING!");
    AVRProgrammer::shutdownTargetMcu();
    setup_test();
    logInfo("SUCCESS!");
    
  } else if (btn == HWInterface::BTN_UPLOAD) {
    #if 0
    char mcuModelBuf[UtilsAVR::MCU_MODEL_BUFFER_SIZE];
    char filePath[Utils::FILE_PATH_BUFFER_SIZE];
    byte signBytes[3];
    AVRProgrammer::readSignatureBytes(signBytes, statusRes);
    if (statusRes != 0) {
      logErrorB("readSignError:", statusRes);
      // FIXME displayError! Display correct one!!!!
      displayError(0xA, 0, 0);
    }
    byte mcuModelId = UtilsAVR::getAVRModelIdBySignature(signBytes, statusRes);
    if (statusRes != 0) {
      logErrorB("getModelIdBySign:", statusRes);
      // FIXME displayError! Display correct one!!!!
      displayError(0xA, 0, 0); return;
    }
    logInfoD("AVR MCU ModelId: ", mcuModelId);
    if (!ConfFile::openConfFile("CONFIG.TXT")) {
      logError("No CONFIG.TXT found!");
    }
    boolean detected = ConfFile::getFilePathByProgIdAndMcuModel(progIdBuf, mcuModelId, mcuModelBuf, filePath, statusRes);
    ConfFile::closeConfFile();
    if (statusRes != 0) {
      logErrorB("getProgramFile:", statusRes);
      // FIXME displayError! Display correct one!!!!
      displayError(0xA, 0, 0); return;
    }
    if (!detected) {
      logError("No MCU ProgramFile found!");
      displayError(0x4,0,0); return;
    }
    logInfoS("ProgramFile: ", filePath);
    // Now run Upload process
    ProgramFile::uploadMcuDataFromFile(filePath, mcuModelId, statusRes);
    #endif
  }

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
