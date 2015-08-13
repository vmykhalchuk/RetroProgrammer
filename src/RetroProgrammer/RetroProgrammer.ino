#include "LoggerA.h"
#include "Utils.h"
#include "AVRConstants.h"
#include "AVRProgrammingRoutines.h"
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
//    A6
const byte pinRandomRead = A6;


//                                    TARGET_MCU_ID__W0, TARGET_MCU_ID__W1, TARGET_MCU_ID__W2, TARGET_MCU_ID__MANUAL0, TARGET_MCU_ID__MANUAL1
TargetProgramDetector programDetector(       A0        ,       A1         ,      A2          ,       A3              ,         A4            );

#define RAM_DEPOSIT_SIZE 550
byte RAM_DEPOSIT[RAM_DEPOSIT_SIZE];

void setup() {
  #if RAM_DEPOSIT_SIZE > 0
    RAM_DEPOSIT[0]=0;
  #endif
  
  AVRProgrammingRoutines_setup();
  Logger_setup();
  TargetProgramDetector_setup();
  //initTargetMcuIdReaderPins();
  //serialInit();
  //lcdInit();
  logInfo("Hello World!");

  //setup_test();
  setup_prod();
}

void setup_test() {//Used for testing
  logFreeRam();

  testUtilsGen();
  testUtilsAVR();
  
  #if 1
    testTargetProgramDetector();
  #endif

  #if 1 // Upoad test files to SD Card!!!
    // Init SD Card
    if (!initSDCard()) {
      logError("SD init failed!");
      return;
    }
    testConfFile();
  #endif

  logInfo("Happy testing day!");
  logFreeRam();
}

void setup_prod() {
  byte statusRes=0;
  
  delay(1000);
  byte b = waitForUserCommand();
  logInfoD("Command:",b);
  
  // Init SD Card
  if (!initSDCard()) {
    logError("SD init failed!");
    return;
  }

  // Prepare Target MCU for Programming
  startupTargetMcuProgramming(statusRes);
  if (statusRes > 0) {
    logError("ProgEn failed!");
    shutdownTargetMcu();
    return;
  } else {
    logInfo("Started prog!");
  }
  delay(2000);


  byte signBytes[3];
  #if 1
  // make sure it is ATmega328P
  readSignatureBytes(signBytes,statusRes);
  if (statusRes > 0) {
    logError("signature error!");
    return;
  }
  delay(2000);
  #endif

  #if 1
  logInfo("Uploading...");
  uploadMcuDataFromFile("TEST01", signBytes,
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
  _testProgramming(statusRes);
  if (statusRes > 0) { logErrorB("Failed!", statusRes); return; }
  logInfo("Success testing!");
  delay(2000);
  #endif
  
  #if 1
  logInfo("Reading...");
  backupMcuDataToFile("RND" + String(analogRead(pinRandomRead),HEX), 
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_64, 
          AVR_MEM_PAGES_COUNT_256, AVR_MEM_PAGE_SIZE_4, statusRes);
  if (statusRes > 0) {
    logError("Error reading!");
    return;
  }
  logInfo("Success reading!");
  delay(2000);
  #endif

  #if 1
  logInfo("Backup to file...");
  backupMcuData("BACK", statusRes);
  if (statusRes > 0) {
    logError("Error backing-up!");
    return;
  }
  logError("Success backing-up!");
  #endif

  // now shutdown Target MCU
  shutdownTargetMcu();

  logInfo("Done!");
}


/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
///    LOOP
///    LOOP
///    LOOP
///    LOOP
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////
/////////////////////////////////////////////

void loop()
{
}
